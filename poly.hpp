
#pragma once
#include <iostream>
#include <bit>

typedef std::uint64_t uint64;

struct Poly {
    uint64 rows[64] = {};

    int left = 0;
    int top = 0;

    int width = 0;
    int height = 0;

    bool get(int x, int y) const {
        return rows[y] & (1ULL << (63 - x));
    }

    void flip(int x, int y) {
        rows[y] ^= 1ULL << (63 - x);
    }

    void update() {
        top = 0;
        for (int y = 0; y < 64; y++) {
            if (rows[y]) { top = y; break; }
        }

        height = 0;
        for (int y = 63; y >= 0; y--) {
            if (rows[y]) { height = y - top + 1; break; }
        }

        uint64 total = 0;
        for (int y = top; y < top + height; y++) total |= rows[y];
        left = total ? std::countl_zero(total) : 0;
        width = total ? 64 - std::countr_zero(total) - left : 0;
    }

    bool fits(const Poly& tile, int px, int py) {
        if (tile.top + py < top || tile.top + tile.height + py > top + height) return false;
        for (int y = tile.top; y < tile.top + tile.height; y++) {
            uint64 shift = px >= 0 ? rows[y + py] << px : rows[y + py] >> -px;
            if ((tile.rows[y] & shift) != tile.rows[y]) return false;
        }
        return true;
    }

    void flip(const Poly& tile, int px, int py) {
        for (int y = tile.top; y < tile.top + tile.height; y++) {
            if (y + py < 0 || y + py > 63) continue;
            uint64 shift = px >= 0 ? tile.rows[y] >> px : tile.rows[y] << -px;
            rows[y + py] ^= shift;
        }
    }

    int roof() const {
        return rows[top] ? std::countl_zero(rows[top]) : 0;
    }

    bool empty() const {
        return width == 0 || height == 0;
    }

    int size() const {
        int total = 0;
        for (int y = top; y < top + height; y++) total += std::popcount(rows[y]);
        return total;
    }

    uint64 hash() {
        uint64 h = 0;
        for (int y = top; y < top + height; y++) {
            uint64 x = rows[y] << left;
            h ^= x + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
            h ^= h >> 30;
            h *= 0xbf58476d1ce4e5b9ULL;
            h ^= h >> 27;
            h *= 0x94d049bb133111ebULL;
            h ^= h >> 31;
        }
        return h;
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

    static Poly Rect(int w, int h) {
        Poly poly;
        uint64 mask = ((1ULL << w) - 1) << (64 - w);
        for (int y = 0; y < h; y++) poly.rows[y] = mask;
        poly.update();
        return poly;
    }

};

inline std::ostream& operator<<(std::ostream& os, const Poly& poly) {
    for (int y = poly.top; y < poly.top + poly.height; y++) {
        for (int x = poly.left; x < poly.left + poly.width; x++) {
            os << (poly.get(x, y) ? '#' : '.');
        }
        os << '\n';
    }
    return os;
}


