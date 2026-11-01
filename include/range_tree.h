#pragma once

// 2D Range Tree over restaurant coordinates.
//
// Primary structure: a perfectly balanced BST on x, built from the
// x-sorted point list (O(n log n)).
// Secondary structure: at every primary node, an AVL tree (avl_tree.h)
// over the y-coordinates of all points in that node's subtree, so a
// y-range can be answered in O(log n + k) at any node.
//
// Construction: built bottom-up. Each node's y-sorted point list is the
// merge of its children's y-sorted lists (O(subtree size) per node), so
// the total construction cost across all nodes is O(n log n), and each
// node's AVL tree is then bulk-built from that sorted list in O(size).
//
// Query: a 2D range query [x1,x2] x [y1,y2] walks down to the split node
// where the search path diverges, then along each side collects O(log n)
// "canonical" subtrees fully inside the x-range; querying each one's AVL
// tree costs O(log n + k_i), giving O(log^2 n + k) overall.

#include <algorithm>
#include <iterator>
#include <vector>

#include "avl_tree.h"
#include "restaurant.h"

class RangeTree2D {
public:
    explicit RangeTree2D(std::vector<Restaurant> points) {
        std::sort(points.begin(), points.end(), [](const Restaurant& a, const Restaurant& b) {
            return a.x < b.x;
        });
        root_ = build(points, 0, static_cast<int>(points.size()) - 1);
    }

    ~RangeTree2D() { destroy(root_); }

    RangeTree2D(const RangeTree2D&) = delete;
    RangeTree2D& operator=(const RangeTree2D&) = delete;

    // All restaurants with x in [x1,x2] and y in [y1,y2].
    std::vector<Restaurant> query(double x1, double x2, double y1, double y2) const {
        std::vector<Restaurant> out;
        queryRec(root_, x1, x2, y1, y2, out);
        return out;
    }

    // Convenience wrapper for a square search window centered at (cx, cy).
    std::vector<Restaurant> queryNearby(double cx, double cy, double halfSide) const {
        return query(cx - halfSide, cx + halfSide, cy - halfSide, cy + halfSide);
    }

private:
    struct Node {
        Restaurant point;   // median point on x used to split
        double xLo, xHi;    // x-range spanned by this node's subtree
        AVLTree yTree;      // secondary structure over y for this subtree
        Node* left;
        Node* right;
        Node(std::vector<Restaurant>&& sortedByY, double lo, double hi)
            : xLo(lo), xHi(hi), yTree(AVLTree::buildBalanced(sortedByY)), left(nullptr), right(nullptr) {}
    };

    Node* root_;

    static std::vector<Restaurant> sortByY(std::vector<Restaurant> pts) {
        std::sort(pts.begin(), pts.end(), [](const Restaurant& a, const Restaurant& b) { return a.y < b.y; });
        return pts;
    }

    static std::vector<Restaurant> mergeByY(const std::vector<Restaurant>& a, const std::vector<Restaurant>& b) {
        std::vector<Restaurant> out;
        out.reserve(a.size() + b.size());
        std::merge(a.begin(), a.end(), b.begin(), b.end(), std::back_inserter(out),
                   [](const Restaurant& p, const Restaurant& q) { return p.y < q.y; });
        return out;
    }

    // Builds the primary (x-balanced) tree over points[lo..hi] (already x-sorted)
    // and returns each built node's y-sorted point list via the return value,
    // so the parent can merge children in O(size) instead of re-sorting.
    struct BuildResult {
        Node* node;
        std::vector<Restaurant> sortedByY;
    };

    Node* build(const std::vector<Restaurant>& xsorted, int lo, int hi) {
        BuildResult r = buildRec(xsorted, lo, hi);
        return r.node;
    }

    BuildResult buildRec(const std::vector<Restaurant>& xsorted, int lo, int hi) {
        if (lo > hi) return {nullptr, {}};
        if (lo == hi) {
            std::vector<Restaurant> single{xsorted[lo]};
            Node* leaf = new Node(std::move(single), xsorted[lo].x, xsorted[lo].x);
            return {leaf, sortByY({xsorted[lo]})};
        }
        int mid = lo + (hi - lo) / 2;
        BuildResult leftRes = buildRec(xsorted, lo, mid);
        BuildResult rightRes = buildRec(xsorted, mid + 1, hi);

        std::vector<Restaurant> merged = mergeByY(leftRes.sortedByY, rightRes.sortedByY);
        Node* node = new Node(std::vector<Restaurant>(merged), xsorted[lo].x, xsorted[hi].x);
        node->left = leftRes.node;
        node->right = rightRes.node;
        return {node, merged};
    }

    static bool fullyInside(const Node* n, double x1, double x2) {
        return n->xLo >= x1 && n->xHi <= x2;
    }

    static bool disjoint(const Node* n, double x1, double x2) {
        return n->xHi < x1 || n->xLo > x2;
    }

    static void queryRec(Node* n, double x1, double x2, double y1, double y2, std::vector<Restaurant>& out) {
        if (!n || disjoint(n, x1, x2)) return;
        if (fullyInside(n, x1, x2)) {
            n->yTree.queryRange(y1, y2, out);
            return;
        }
        queryRec(n->left, x1, x2, y1, y2, out);
        queryRec(n->right, x1, x2, y1, y2, out);
    }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }
};
