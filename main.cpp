
#include <unordered_set>
#include <unordered_map>
#include <vector>
#include <chrono>
#include <numeric>
#include "poly.hpp"

typedef std::unordered_map<uint64, uint64> HT;
typedef std::unordered_set<uint64> HS;

uint64 count(Poly& poly, const std::vector<Poly>& tiles, HT& memo) {
    if (poly.empty()) return 1;

    uint64 hash = poly.hash();
    auto it = memo.find(hash);
    if (it != memo.end()) return it -> second;
   
    uint64 total = 0;
    for (const Poly& tile : tiles) {
        int px = poly.roof() - tile.roof();
        int py = poly.top - tile.top;

        bool fits = poly.fits(tile, px, py);

        if (fits) {
            poly.flip(tile, px, py);
            poly.update();
            total += count(poly, tiles, memo);
            poly.flip(tile, px, py);
            poly.update();
        }
    }

    memo[hash] = total;
    return total;
}

bool count_eq(Poly& poly, const std::vector<Poly>& tiles, uint64 n, HT& memo) {
    if (poly.empty()) return n == 1;

    uint64 hash = poly.hash();
    auto it = memo.find(hash);
    if (it != memo.end()) return (it -> second) == n;
   
    uint64 total = 0;
  
    for (const Poly& tile : tiles) {
        int px = poly.roof() - tile.roof();
        int py = poly.top - tile.top;

        bool fits = poly.fits(tile, px, py);

        if (fits) {
            poly.flip(tile, px, py);
            poly.update();
            total += count(poly, tiles, memo);
            poly.flip(tile, px, py);
            poly.update();
        }

        if (total > n) return false;
    }

    memo[hash] = total;
    return total == n;
}


uint64 count(const Poly& p, const std::vector<Poly>& tiles) {
    HT memo = {};
    Poly poly = p;
    return count(poly, tiles, memo);
}

bool count_eq(const Poly& p, const std::vector<Poly>& tiles, uint64 n) {
    HT memo = {};
    Poly poly = p;
    return count_eq(poly, tiles, n, memo);
}

void perft() {
    std::vector<Poly> tiles = {
        Poly::Rect(1,2),
        Poly::Rect(2,1)
    };
  
    for (int i = 2; i <= 20; i += 2) {
        Poly region = Poly::Rect(i, i);
        auto start = std::chrono::steady_clock::now();
        uint64 c = count(region, tiles);
        auto end = std::chrono::steady_clock::now();
        auto t = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        std::print("{0}x{0}: {1} ({2}μs)\n", i, c, t);
    }

}

void expand_h(const Poly& p, std::vector<Poly>& out, HS& memo) {
    Poly poly = p;
    if (poly.left == 0) {
        for (int y = poly.top; y < poly.top + poly.height; y++) poly.rows[y] >>= 1;
        poly.update();
    }
    for (int y = poly.top; y < poly.top + poly.height; y++) {
        uint64 row = poly.rows[y];
        uint64 mask = ((row << 1) | (row >> 1)) & (~row);

        while (mask) {
            uint64 bit = mask & -mask;
            mask ^= bit;

            Poly poly2 = poly;
            poly2.rows[y] ^= bit;
            poly2.update();

            uint64 hash = poly2.hash();
            if (memo.contains(hash)) continue;
            memo.insert(hash);
            out.push_back(poly2);
        }
    }
}


void expand_v(const Poly& poly, std::vector<Poly>& out, HS& memo) {
    for (int y = poly.top; y <= poly.top + poly.height; y++) {
        if (y >= 64) break;
        uint64 up = (y == 0) ? 0 : poly.rows[y-1];
        uint64 down = (y == 63) ? 0 : poly.rows[y+1];
        uint64 mask = (up | down) & (~poly.rows[y]);

        while (mask) {
            uint64 bit = mask & -mask;
            mask ^= bit;

            Poly poly2 = poly;
            poly2.rows[y] ^= bit;
            poly2.update();

            uint64 hash = poly2.hash();
            if (memo.contains(hash)) continue;
            memo.insert(hash);
            out.push_back(poly2);
        }
    }
}


void all_poly(int k, std::vector<Poly>& out) {
    if (k == 1) {
        out.push_back(Poly::Rect(1, 1));
        return;
    }

    std::vector<Poly> prev;
    all_poly(k - 1, prev);

    HS memo = {};

    for (const Poly& poly : prev) {
        expand_h(poly, out, memo);
        expand_v(poly, out, memo);
    }
}

Poly min_area(std::vector<Poly>& tiles, uint64 n) {
    int gcd = tiles[0].size();
    for (const Poly& tile : tiles) gcd = std::gcd(gcd, tile.size());

    int k = 1;
    while (true) {
        std::vector<Poly> polys;
        all_poly(k * gcd, polys);
        for (const Poly& poly : polys) if (count_eq(poly, tiles, n)) return poly;
        k += 1;
    }
}

Poly blur(Poly& poly) {
    Poly res = poly;
    for (int y = poly.top; y < poly.top + poly.height; y++) {
        uint64 h = (poly.rows[y] << 1) | (poly.rows[y] >> 1);
        uint64 v = (y > 0 ? poly.rows[y-1] : 0) | (y < 63 ? poly.rows[y+1] : 0);
        res.rows[y] |= (h | v);
    }
    res.update();
    return res;
}

// void grow(Poly& poly, Poly& r, Poly& R, int n, std::vector<Poly>& out) {
//     Poly b = blur(poly);
// }

int main() {
    // perft();
    // for (int n = 1; n < 20; n++) {
    //     std::vector<Poly> out;
    //     all_poly(n, out);
    //
    //     // std::print("Area = {0}:\n", n);
    //     // for (Poly& poly : out) std::cout << poly.hash() << std::endl << poly << std::endl;
    //     std::print("{0}: {1}\n", n, out.size());
    // }
    //
    
    // std::vector<Poly> tiles;
    // Poly poly = Poly::Rect(2,2);
    // poly.flip(0,0);
    // tiles.push_back(poly);
    // poly.flip(0,0);
    // poly.flip(1,0);
    // tiles.push_back(poly);
    // poly.flip(1,0);
    // poly.flip(0,1);
    // tiles.push_back(poly);
    // poly.flip(0,1);
    // poly.flip(1,1);
    // tiles.push_back(poly);

    // std::vector<Poly> tiles = { Poly::Rect(2,1), Poly::Rect(1,2) };
    
    // for (int n = 1; n < 20; n++) {
    //     Poly poly = min_area(tiles, n);
    //     std::cout << n << ": " << poly.size() << std::endl << poly << std::endl;
    // }

    for (int n = 1; n < 20; n++) {
        std::vector<Poly> out;
        all_poly(n, out);
        std::print("{0}: {1}\n", n, out.size());
    }

    // Poly poly = Poly::Rect(5,2);
    //
    // std::cout << poly << std::endl;
    // poly.trans();
    // poly.update();
    // std::cout << poly << std::endl;
}
