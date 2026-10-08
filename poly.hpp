
#pragma once

#include <iostream>
#include <bit>
#include <unordered_set>
#include <unordered_map>

typedef std::uint64_t uint64;
typedef std::uint8_t uint8;

typedef std::unordered_map<uint64, uint64> HT;
typedef std::unordered_set<uint64> HS;

inline uint64 reverse_bits(uint64_t x) {
    x = ((x & 0x5555555555555555ULL) << 1) | ((x >> 1) & 0x5555555555555555ULL);
    x = ((x & 0x3333333333333333ULL) << 2) | ((x >> 2) & 0x3333333333333333ULL);
    x = ((x & 0x0F0F0F0F0F0F0F0FULL) << 4) | ((x >> 4) & 0x0F0F0F0F0F0F0F0FULL);
    x = ((x & 0x00FF00FF00FF00FFULL) << 8) | ((x >> 8) & 0x00FF00FF00FF00FFULL);
    x = ((x & 0x0000FFFF0000FFFFULL) << 16) | ((x >> 16) & 0x0000FFFF0000FFFFULL);
    return (x << 32) | (x >> 32);
}

struct Poly {
    uint64 rows[64] = {};

    uint8 left = 0;
    uint8 right = 0;

    uint8 top = 0;
    uint8 bottom = 0;
 
    bool get(uint8 x, uint8 y) const {
        return rows[y] & (1ULL << (63 - x));
    }

    void update_top(uint8 t = 0) {
        top = 0;
        for (uint8 y = t; y < 64; y++) if (rows[y]) { top = y; break; }
    }

    void update_bottom(uint8 b = 0) {
        bottom = 0;
        for (uint8 y = b; y < 64; y++) if (rows[63 - y]) { bottom = y; break; }
    }

    void update_left_right() {
        uint64 total = 0;
        for (uint8 y = top; y < 64 - bottom; y++) total |= rows[y];
        left = total ? std::countl_zero(total) : 0;
        right = total ? std::countr_zero(total) : 0;
    }

    void update() {
        update_top();
        update_bottom();
        update_left_right();
    }

    bool fits(const Poly& tile, int px, int py) const {
        if (tile.top + py < top || 64 - tile.bottom + py > 64 - bottom) return false;
        for (uint8 y = tile.top; y < 64 - tile.bottom; y++) {
            uint64 shift = px >= 0 ? rows[y + py] << px : rows[y + py] >> -px;
            if ((tile.rows[y] & shift) != tile.rows[y]) return false;
        }
        return true;
    }

    void flip(uint8 x, uint8 y) {
        rows[y] ^= 1ULL << (63 - x);
    }

    void set(const Poly& tile, int px, int py) {
        for (uint8 y = tile.top; y < 64 - tile.bottom; y++) {
            if (y + py < 0 || y + py > 63) continue;
            uint64 shift = px >= 0 ? tile.rows[y] >> px : tile.rows[y] << -px;
            rows[y + py] |= shift;

            left = std::min(left, uint8(std::countl_zero(shift)));
            right = std::min(right, uint8(std::countr_zero(shift)));
        }

        bottom = std::min(bottom, uint8(tile.bottom - py));
        top = std::min(top, uint8(tile.top + py));
    }

     void unset(const Poly& tile, int px, int py) {
        for (uint8 y = tile.top; y < 64 - tile.bottom; y++) {
            if (y + py < 0 || y + py > 63) continue;
            uint64 shift = px >= 0 ? tile.rows[y] >> px : tile.rows[y] << -px;
            rows[y + py] &= (~shift);
        }

        if (tile.bottom - py == bottom) update_bottom(bottom);
        if (tile.top + py == top) update_top(top);

        update_left_right();
    }

    int roof() const {
        return rows[top] ? std::countl_zero(rows[top]) : 0;
    }

    bool empty() const {
        return left == 0 && right == 0;
    }

    uint64 size() const {
        uint64 total = 0;
        for (int y = top; y < 64 - bottom; y++) total += std::popcount(rows[y]);
        return total;
    }

    uint64 hash() const {
        uint64 h1 = 0x7485736ef72091afULL;
        for (int y = top; y < 64 - bottom; y++) {
            uint64 x = rows[y] << left;
            h1 ^= x + 0x9e3779b97f4a7c15ULL + (h1 << 6) + (h1 >> 2);
            h1 ^= h1 >> 30;
            h1 *= 0xbf58476d1ce4e5b9ULL;
            h1 ^= h1 >> 27;
            h1 *= 0x94d049bb133111ebULL;
            h1 ^= h1 >> 31;
        }

        // uint8 right = 64 - left - width;
        // uint64 h2 = 0x7485736ef72091afULL;
        // for (int y = top; y < top + height; y++) {
        //     uint64 x = reverse_bits(rows[y]) << right;
        //     h2 ^= x + 0x9e80f9b783ea7c13ULL + (h2 << 6) + (h2 >> 2);
        //     h2 ^= h2 >> 30;
        //     h2 *= 0x7849ea83e572ff30ULL;
        //     h2 ^= h2 >> 27;
        //     h2 *= 0x74839e920a928f8cULL;
        //     h2 ^= h2 >> 31;
        // }

        // return h1 ^ h2;

        return h1;
    }

    void trans() {
        static const uint64 mask[6][2] = {
            {0X5555555555555555, 0XAAAAAAAAAAAAAAAA},
            {0X3333333333333333, 0XCCCCCCCCCCCCCCCC},
            {0X0F0F0F0F0F0F0F0F, 0XF0F0F0F0F0F0F0F0},
            {0X00FF00FF00FF00FF, 0XFF00FF00FF00FF00},
            {0X0000FFFF0000FFFF, 0XFFFF0000FFFF0000},
            {0X00000000FFFFFFFF, 0XFFFFFFFF00000000}
        };

        int i, j, p, s;
        uint64_t idx0, idx1, x, y;

        for (j = 5; j >= 0; j--) {
            s = 1 << j;
            for (p = 0; p < 32/s; p++) {
                for (i = 0; i < s; i++) {
                    idx0 = p*2*s + i;
                    idx1 = p*2*s + i + s;
                    x = (rows[idx0] & mask[j][0]) | ((rows[idx1] & mask[j][0]) << s);
                    y = ((rows[idx0] & mask[j][1]) >> s) | (rows[idx1] & mask[j][1]);
                    rows[idx0] = x;
                    rows[idx1] = y;
                }
            }
        }
    }

    static Poly Rect(uint8 w, uint8 h) {
        Poly poly;
        uint64 mask = ((1ULL << w) - 1) << (64 - w);
        for (uint8 y = 0; y < h; y++) poly.rows[y] = mask;
        poly.update();
        return poly;
    }

};

inline std::ostream& operator<<(std::ostream& os, const Poly& poly) {
    for (int y = poly.top; y < 64 - poly.bottom; y++) {
        for (int x = poly.left; x < 64 - poly.right; x++) {
            os << (poly.get(x, y) ? '#' : '.');
        }
        os << '\n';
    }
    return os;
}


