#include "../include/HuffmanTree.h"
#include <queue>
#include <vector>
#include <algorithm>
#include <stdexcept>

// ---------------------------------------------------------------------------
HuffmanTree::HuffmanTree() : root_(nullptr) {}

HuffmanTree::~HuffmanTree() {
    destroyTree(root_);
}

void HuffmanTree::destroyTree(HuffmanNode* node) {
    if (!node) return;
    destroyTree(node->left);
    destroyTree(node->right);
    delete node;
}

// ---------------------------------------------------------------------------
// build — construct Huffman tree using a min-heap priority queue.
//
// Determinism guarantee:
//   unordered_map iteration order is non-deterministic across runs.
//   To ensure compress and decompress always build the IDENTICAL tree,
//   we sort the frequency entries by (freq ASC, char ASC) before pushing
//   them into the heap, assigning a monotonically increasing `order`.
//   The NodeComparator breaks frequency ties by `order`, so the push
//   sequence is fully reproducible from any unordered_map with the same
//   entries.
//
// Algorithm (Greedy):
//   1. Push every character as a leaf node into a min-heap.
//   2. While heap size > 1:
//        a. Pop two nodes with the smallest frequencies (A, B).
//        b. Create an internal node with freq = A->freq + B->freq,
//           order = next counter value.
//        c. Push internal node back.
//   3. The remaining node is the root.
// ---------------------------------------------------------------------------
void HuffmanTree::build(const std::unordered_map<char, int>& freqTable) {
    if (freqTable.empty())
        throw std::runtime_error("Frequency table is empty — nothing to compress.");

    destroyTree(root_);
    root_ = nullptr;
    codes_.clear();
    freqTable_ = freqTable;

    // Collect and sort entries: primary by freq ASC, secondary by char ASC
    // (same total ordering used by the comparator so initial push order = pop order)
    std::vector<std::pair<int, char>> entries;
    entries.reserve(freqTable.size());
    for (const auto& [ch, freq] : freqTable)
        entries.emplace_back(freq, ch);
    std::sort(entries.begin(), entries.end(),
              [](const std::pair<int,char>& a, const std::pair<int,char>& b) {
                  if (a.first != b.first) return a.first < b.first;
                  return (unsigned char)a.second < (unsigned char)b.second;
              });

    int orderCounter = 0;

    std::priority_queue<HuffmanNode*, std::vector<HuffmanNode*>, NodeComparator> minHeap;

    for (const auto& [freq, ch] : entries)
        minHeap.push(new HuffmanNode(ch, freq, orderCounter++));

    // Edge case: single unique character
    if (minHeap.size() == 1) {
        HuffmanNode* only = minHeap.top(); minHeap.pop();
        root_ = new HuffmanNode(only->frequency, orderCounter++, only, nullptr);
    } else {
        while (minHeap.size() > 1) {
            HuffmanNode* left  = minHeap.top(); minHeap.pop();
            HuffmanNode* right = minHeap.top(); minHeap.pop();
            HuffmanNode* merged = new HuffmanNode(
                left->frequency + right->frequency,
                orderCounter++,
                left, right);
            minHeap.push(merged);
        }
        root_ = minHeap.top();
    }

    generateCodes(root_, "");
}

void HuffmanTree::rebuild(const std::unordered_map<char, int>& freqTable) {
    build(freqTable);
}

// ---------------------------------------------------------------------------
// generateCodes — DFS (pre-order) traversal
// ---------------------------------------------------------------------------
void HuffmanTree::generateCodes(HuffmanNode* node, const std::string& prefix) {
    if (!node) return;
    if (node->isLeaf()) {
        codes_[node->ch] = prefix.empty() ? "0" : prefix;
        return;
    }
    generateCodes(node->left,  prefix + '0');
    generateCodes(node->right, prefix + '1');
}

const std::unordered_map<char, std::string>& HuffmanTree::getCodes() const {
    return codes_;
}

const HuffmanNode* HuffmanTree::getRoot() const {
    return root_;
}

int HuffmanTree::computeHeight(const HuffmanNode* node) const {
    if (!node) return 0;
    return 1 + std::max(computeHeight(node->left), computeHeight(node->right));
}

int HuffmanTree::height() const {
    return computeHeight(root_);
}

double HuffmanTree::averageCodeLength() const {
    int totalChars = totalCharacters();
    if (totalChars == 0) return 0.0;
    double weightedSum = 0.0;
    for (const auto& [ch, code] : codes_) {
        auto it = freqTable_.find(ch);
        if (it != freqTable_.end())
            weightedSum += static_cast<double>(it->second) * code.size();
    }
    return weightedSum / totalChars;
}

int HuffmanTree::totalCharacters() const {
    int total = 0;
    for (const auto& [ch, freq] : freqTable_)
        total += freq;
    return total;
}
