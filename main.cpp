
#include <chrono>
#include <vector>
#include "poly.hpp"

struct TileData {
    uint64 hash;
    uint8 roof;
};

uint64 count(Poly& poly, const std::vector<Poly>& tiles, HT& memo, std::vector<TileData>& tileData) {
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
    for (size_t i = 0; i < tiles.size(); i++) {
        const Poly& tile = tiles[i];
        if (tileData[i].hash == hash) return 1;

        int px = poly.roof() - tileData[i].roof;
        int py = poly.top - tile.top;

        bool fits = poly.fits(tile, px, py);

        if (fits) {
            poly.unset(tile, px, py);
            total += count(poly, tiles, memo, tileData);
            poly.set(tile, px, py);
        }
    }

    memo[hash] = total;
    return total;
}

uint64 count(const Poly& p, const std::vector<Poly>& tiles) {
    HT memo = {};
    memo.reserve(1000000);

    std::vector<TileData> tileData;
    for (size_t i = 0; i < tiles.size(); i++) tileData.emplace_back(tiles[i].hash(), tiles[i].roof());

    Poly poly = p;
    return count(poly, tiles, memo, tileData);
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
    // Poly poly = Poly::Rect(2,2);
    // poly.unset(Poly::Rect(2,1), 0, 0);
    // poly.update();

    // std::cout << poly << std::endl << int(poly.left) << ", " << int(poly.width) << ", " << int(poly.top) << ", " << int(poly.height) << std::endl;
}
