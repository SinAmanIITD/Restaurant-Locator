#include <chrono>
#include <iomanip>
#include <iostream>
#include <random>
#include <vector>

#include "range_tree.h"
#include "restaurant.h"

namespace {

std::vector<Restaurant> generateRandomRestaurants(int n, unsigned seed = 42) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> coord(0.0, 10000.0);
    std::vector<Restaurant> pts;
    pts.reserve(n);
    for (int i = 0; i < n; ++i) {
        pts.emplace_back(i, "Restaurant_" + std::to_string(i), coord(rng), coord(rng));
    }
    return pts;
}

}  // namespace

int main() {
    std::cout << "=== Restaurant Locator: 2D Range Tree over AVL Trees ===\n\n";

    const int n = 20000;
    std::vector<Restaurant> restaurants = generateRandomRestaurants(n);

    auto buildStart = std::chrono::high_resolution_clock::now();
    RangeTree2D tree(restaurants);
    auto buildEnd = std::chrono::high_resolution_clock::now();
    double buildMs = std::chrono::duration<double, std::milli>(buildEnd - buildStart).count();

    std::cout << "Indexed " << n << " restaurants in " << std::fixed << std::setprecision(2)
              << buildMs << " ms (O(n log n) preprocessing)\n\n";

    // Example: find restaurants within a 500x500 square centered at (5000, 5000).
    double cx = 5000.0, cy = 5000.0, halfSide = 250.0;

    auto queryStart = std::chrono::high_resolution_clock::now();
    std::vector<Restaurant> nearby = tree.queryNearby(cx, cy, halfSide);
    auto queryEnd = std::chrono::high_resolution_clock::now();
    double queryUs = std::chrono::duration<double, std::micro>(queryEnd - queryStart).count();

    std::cout << "Square query centered at (" << cx << ", " << cy << "), half-side " << halfSide
              << "\nFound " << nearby.size() << " restaurants in " << queryUs << " us\n\n";

    std::cout << "Sample results:\n";
    int shown = 0;
    for (const auto& r : nearby) {
        if (shown++ >= 5) break;
        std::cout << "  " << r.name << " @ (" << r.x << ", " << r.y << ")\n";
    }

    return 0;
}
