
#include "poly.hpp"
#include <vector>

void inline expand_h(const Poly& p, std::vector<Poly>& out, HS& memo) {
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

void inline expand_v(const Poly& poly, std::vector<Poly>& out, HS& memo) {
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

void inline all_poly(int k, std::vector<Poly>& out) {
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

// Poly min_area(std::vector<Poly>& tiles, uint64 n) {
//     int gcd = tiles[0].size();
//     for (const Poly& tile : tiles) gcd = std::gcd(gcd, tile.size());
//
//     int k = 1;
//     while (true) {
//         std::vector<Poly> polys;
//         all_poly(k * gcd, polys);
//         for (const Poly& poly : polys) if (count_eq(poly, tiles, n)) return poly;
//         k += 1;
//     }
// }

// Poly blur(Poly& poly) {
//     Poly res = poly;
//     for (int y = poly.top; y < poly.top + poly.height; y++) {
//         uint64 h = (poly.rows[y] << 1) | (poly.rows[y] >> 1);
//         uint64 v = (y > 0 ? poly.rows[y-1] : 0) | (y < 63 ? poly.rows[y+1] : 0);
//         res.rows[y] |= (h | v);
//     }
//     res.update();
//     return res;
// }

// void grow(Poly& poly, Poly& r, Poly& R, int n, std::vector<Poly>& out) {
//     Poly b = blur(poly);
// }
