#pragma once

// ---------------------------------------------------------------------------
// HuffmanNode
// Represents a single node in the Huffman Tree.
// Leaf nodes hold a real character; internal nodes hold ch = '\0'.
// ---------------------------------------------------------------------------
struct HuffmanNode {
    char     ch;
    int      frequency;
    int      order;       // insertion order — used as tie-breaker in the heap
    HuffmanNode* left  = nullptr;
    HuffmanNode* right = nullptr;

    // Leaf node constructor
    HuffmanNode(char c, int freq, int ord)
        : ch(c), frequency(freq), order(ord), left(nullptr), right(nullptr) {}

    // Internal node constructor (merged node)
    HuffmanNode(int freq, int ord, HuffmanNode* l, HuffmanNode* r)
        : ch('\0'), frequency(freq), order(ord), left(l), right(r) {}

    bool isLeaf() const { return left == nullptr && right == nullptr; }
};

// ---------------------------------------------------------------------------
// Comparator for the min-heap priority queue.
// Primary  : lower frequency  = higher priority
// Secondary: lower order (earlier inserted) = higher priority
// Using insertion order as tie-breaker gives a fully deterministic tree
// regardless of the iteration order of the unordered_map.
// ---------------------------------------------------------------------------
struct NodeComparator {
    bool operator()(const HuffmanNode* a, const HuffmanNode* b) const {
        if (a->frequency != b->frequency)
            return a->frequency > b->frequency;
        return a->order > b->order;
    }
};
