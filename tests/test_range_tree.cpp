// Correctness tests (vs. brute force) and an empirical complexity benchmark
// for the 2D range tree, run as a plain executable (no external framework).

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <iostream>
#include <random>
#include <vector>

#include "range_tree.h"
#include "restaurant.h"

namespace {

std::vector<Restaurant> randomPoints(int n, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> coord(0.0, 1000.0);
    std::vector<Restaurant> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) pts.emplace_back(i, "r" + std::to_string(i), coord(rng), coord(rng));
    return pts;
}

std::vector<Restaurant> bruteForceQuery(const std::vector<Restaurant>& pts, double x1, double x2, double y1, double y2) {
    std::vector<Restaurant> out;
    for (const auto& p : pts) {
        if (p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2) out.push_back(p);
    }
    return out;
}

bool sameSet(std::vector<Restaurant> a, std::vector<Restaurant> b) {
    auto byId = [](const Restaurant& p, const Restaurant& q) { return p.id < q.id; };
    std::sort(a.begin(), a.end(), byId);
    std::sort(b.begin(), b.end(), byId);
    if (a.size() != b.size()) return false;
    for (size_t i = 0; i < a.size(); ++i)
        if (a[i].id != b[i].id) return false;
    return true;
}

void testCorrectnessRandomized() {
    std::mt19937 rng(123);
    std::uniform_real_distribution<double> coord(0.0, 1000.0);

    std::vector<Restaurant> pts = randomPoints(2000, 7);
    RangeTree2D tree(pts);

    for (int trial = 0; trial < 200; ++trial) {
        double a = coord(rng), b = coord(rng);
        double c = coord(rng), d = coord(rng);
        double x1 = std::min(a, b), x2 = std::max(a, b);
        double y1 = std::min(c, d), y2 = std::max(c, d);

        auto expected = bruteForceQuery(pts, x1, x2, y1, y2);
        auto actual = tree.query(x1, x2, y1, y2);
        assert(sameSet(expected, actual) && "range tree result mismatched brute force");
    }
    std::cout << "[PASS] testCorrectnessRandomized (200 random queries vs brute force)\n";
}

void testEdgeCases() {
    std::vector<Restaurant> pts = {
        {0, "a", 0, 0}, {1, "b", 10, 10}, {2, "c", 5, 5}, {3, "d", -5, -5}, {4, "e", 5, -5},
    };
    RangeTree2D tree(pts);

    // Single point exact match.
    auto exact = tree.query(5, 5, 5, 5);
    assert(exact.size() == 1 && exact[0].id == 2);

    // Empty result.
    auto empty = tree.query(100, 200, 100, 200);
    assert(empty.empty());

    // Full range.
    auto all = tree.query(-100, 100, -100, 100);
    assert(all.size() == pts.size());

    std::cout << "[PASS] testEdgeCases\n";
}

void benchmarkComplexity() {
    std::cout << "\n--- Empirical scaling (query time should grow roughly as log^2 n) ---\n";
    for (int n : {1000, 2000, 4000, 8000, 16000, 32000}) {
        std::vector<Restaurant> pts = randomPoints(n, 99);
        RangeTree2D tree(pts);

        const int numQueries = 500;
        std::mt19937 rng(1);
        std::uniform_real_distribution<double> coord(0.0, 1000.0);

        auto start = std::chrono::high_resolution_clock::now();
        long long totalResults = 0;
        for (int i = 0; i < numQueries; ++i) {
            double cx = coord(rng), cy = coord(rng);
            auto res = tree.queryNearby(cx, cy, 20.0);
            totalResults += static_cast<long long>(res.size());
        }
        auto end = std::chrono::high_resolution_clock::now();
        double avgUs = std::chrono::duration<double, std::micro>(end - start).count() / numQueries;

        double log2n = std::log2(static_cast<double>(n));
        std::cout << "n=" << n << "\tavg query=" << avgUs << " us\tlog2(n)=" << log2n
                   << "\tlog2(n)^2=" << (log2n * log2n) << "\tavg results=" << (totalResults / (double)numQueries) << "\n";
    }
}

}  // namespace

int main() {
    testCorrectnessRandomized();
    testEdgeCases();
    benchmarkComplexity();
    std::cout << "\nAll tests passed.\n";
    return 0;
}
