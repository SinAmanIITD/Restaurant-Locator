# Restaurant Locator

Efficient nearby-restaurant queries over 2D coordinates using a **2D Range
Tree** whose secondary (per-node) structure is an **AVL tree**, implemented
from scratch in C++17.

## How it works

- **Primary structure**: a perfectly balanced BST over restaurants' x-coordinates,
  built in O(n log n) from the x-sorted point list.
- **Secondary structure**: every primary-tree node stores an AVL tree of the
  y-coordinates of all points in its subtree. These are built bottom-up by
  merging each node's children's already y-sorted lists (O(subtree size) per
  node), then bulk-loading a perfectly balanced AVL tree from that sorted list
  in O(size) — keeping total preprocessing at **O(n log n)** instead of the
  naive O(n log² n) you'd get from re-inserting points one at a time.
- **Query**: a rectangular range `[x1,x2] x [y1,y2]` (or a square window via
  `queryNearby`) splits the primary tree into O(log n) canonical subtrees fully
  inside the x-range, then runs an O(log n + k) range query on each one's AVL
  tree — giving **O(log² n + k)** overall, where k is the number of results.

## Project layout

```
include/
  restaurant.h    - point/record type
  avl_tree.h       - templated AVL tree, insert + O(log n) range query + O(n) balanced bulk-build
  range_tree.h     - 2D range tree combining both structures
src/main.cpp       - demo: indexes 20,000 restaurants, runs a square-range query
tests/test_range_tree.cpp - correctness tests (vs. brute force) + empirical complexity benchmark
```

## Build & run

```bash
make run    # builds and runs the demo (src/main.cpp)
make test   # builds and runs correctness tests + complexity benchmark
```

## Verified results (Apple M2, -O2)

Correctness: 200 randomized range queries checked against an O(n) brute-force
scan, plus edge cases (empty range, single-point match, full range) — all pass.

Demo run: indexing 20,000 restaurants takes ~30 ms; a 500x500 square query
over that index returns in ~19 microseconds.

Empirical query-time scaling (500 random square queries averaged per n),
which tracks the expected O(log² n) growth:

| n      | avg query time | log2(n)^2 |
|--------|----------------|-----------|
| 1,000  | 0.76 us        | 99.3      |
| 2,000  | 1.05 us        | 120.2     |
| 4,000  | 1.26 us        | 143.2     |
| 8,000  | 1.60 us        | 168.1     |
| 16,000 | 2.39 us        | 195.0     |
| 32,000 | 3.64 us        | 224.0     |
