#pragma once

// AVL tree keyed on y-coordinate, used as the associated (secondary)
// structure at each node of the 2D range tree. Supports O(log n) range
// queries over y, and can be bulk-built from an already y-sorted vector
// in O(n) time (perfectly balanced build), which is what keeps the
// overall range-tree construction at O(n log n) instead of O(n log^2 n).

#include <algorithm>
#include <vector>

#include "restaurant.h"

class AVLTree {
public:
    AVLTree() : root_(nullptr) {}
    ~AVLTree() { destroy(root_); }

    AVLTree(const AVLTree&) = delete;
    AVLTree& operator=(const AVLTree&) = delete;

    AVLTree(AVLTree&& other) noexcept : root_(other.root_) { other.root_ = nullptr; }
    AVLTree& operator=(AVLTree&& other) noexcept {
        if (this != &other) {
            destroy(root_);
            root_ = other.root_;
            other.root_ = nullptr;
        }
        return *this;
    }

    void insert(const Restaurant& r) { root_ = insertNode(root_, r); }

    // Builds a perfectly balanced AVL tree from points already sorted by y.
    // O(n) time since no rotations are needed when built this way.
    static AVLTree buildBalanced(const std::vector<Restaurant>& sortedByY) {
        AVLTree tree;
        tree.root_ = buildBalancedRec(sortedByY, 0, static_cast<int>(sortedByY.size()) - 1);
        return tree;
    }

    // Returns all points with y in [yLo, yHi], O(log n + k) where k = #results.
    void queryRange(double yLo, double yHi, std::vector<Restaurant>& out) const {
        queryRangeRec(root_, yLo, yHi, out);
    }

    int size() const { return countNodes(root_); }

private:
    struct Node {
        Restaurant data;
        Node* left;
        Node* right;
        int height;
        explicit Node(const Restaurant& r) : data(r), left(nullptr), right(nullptr), height(1) {}
    };

    Node* root_;

    static int height(Node* n) { return n ? n->height : 0; }
    static int balanceFactor(Node* n) { return n ? height(n->left) - height(n->right) : 0; }
    static void updateHeight(Node* n) { n->height = 1 + std::max(height(n->left), height(n->right)); }

    static Node* rotateRight(Node* y) {
        Node* x = y->left;
        Node* t2 = x->right;
        x->right = y;
        y->left = t2;
        updateHeight(y);
        updateHeight(x);
        return x;
    }

    static Node* rotateLeft(Node* x) {
        Node* y = x->right;
        Node* t2 = y->left;
        y->left = x;
        x->right = t2;
        updateHeight(x);
        updateHeight(y);
        return y;
    }

    static Node* rebalance(Node* n) {
        updateHeight(n);
        int bf = balanceFactor(n);
        if (bf > 1) {
            if (balanceFactor(n->left) < 0) n->left = rotateLeft(n->left);
            return rotateRight(n);
        }
        if (bf < -1) {
            if (balanceFactor(n->right) > 0) n->right = rotateRight(n->right);
            return rotateLeft(n);
        }
        return n;
    }

    static Node* insertNode(Node* n, const Restaurant& r) {
        if (!n) return new Node(r);
        if (r.y < n->data.y) {
            n->left = insertNode(n->left, r);
        } else {
            n->right = insertNode(n->right, r);
        }
        return rebalance(n);
    }

    static Node* buildBalancedRec(const std::vector<Restaurant>& pts, int lo, int hi) {
        if (lo > hi) return nullptr;
        int mid = lo + (hi - lo) / 2;
        Node* n = new Node(pts[mid]);
        n->left = buildBalancedRec(pts, lo, mid - 1);
        n->right = buildBalancedRec(pts, mid + 1, hi);
        updateHeight(n);
        return n;
    }

    static void queryRangeRec(Node* n, double yLo, double yHi, std::vector<Restaurant>& out) {
        if (!n) return;
        if (n->data.y > yLo) queryRangeRec(n->left, yLo, yHi, out);
        if (n->data.y >= yLo && n->data.y <= yHi) out.push_back(n->data);
        if (n->data.y < yHi) queryRangeRec(n->right, yLo, yHi, out);
    }

    static int countNodes(Node* n) { return n ? 1 + countNodes(n->left) + countNodes(n->right) : 0; }

    static void destroy(Node* n) {
        if (!n) return;
        destroy(n->left);
        destroy(n->right);
        delete n;
    }
};
