
#include <chrono>
#include <vector>
#include "poly.hpp"

uint64 count(Poly& poly, const std::vector<Poly>& tiles, HT& memo) {
    // if (poly.empty()) return 1;

    uint64 hash = poly.hash();
    auto it = memo.find(hash);
    if (it != memo.end()) return it -> second;

    // poly.trans();
    // uint64 hash2 = poly.hash();
    // poly.trans();
    //
    // auto it2 = memo.find(hash2);
    // if (it2 != memo.end()) return it -> second;
    //
    uint64 total = 0;
    for (const Poly& tile : tiles) {
        if (tile.hash() == hash) return 1;

        int px = poly.roof() - tile.roof();
        int py = poly.top - tile.top;

        bool fits = poly.fits(tile, px, py);

        if (fits) {
            poly.unset(tile, px, py);
            total += count(poly, tiles, memo);
            poly.set(tile, px, py);
        }
    }

    memo[hash] = total;
    return total;
}

uint64 count(const Poly& p, const std::vector<Poly>& tiles) {
    HT memo = {};
    Poly poly = p;
    return count(poly, tiles, memo);
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

int main() {
    perft(); 
    // Poly poly = {};
    // poly.update();
    // Poly domino = Poly::Rect(2,1);
    //
    // poly.set(domino, 3, 5);
    // std::cout << poly << std::endl << poly.left << ", " << poly.width << ", " << poly.top << ", " << poly.height << std::endl;
}
