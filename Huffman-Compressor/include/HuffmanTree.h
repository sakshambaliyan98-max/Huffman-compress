#pragma once

#include "HuffmanNode.h"
#include <unordered_map>
#include <string>

// ---------------------------------------------------------------------------
// HuffmanTree
// Owns the tree memory.  Builds the tree from a frequency table and
// generates/provides Huffman codes via a flat map.
// ---------------------------------------------------------------------------
class HuffmanTree {
public:
    HuffmanTree();
    ~HuffmanTree();

    // Build tree from a frequency map {char -> count}
    void build(const std::unordered_map<char, int>& freqTable);

    // Rebuild tree from a stored frequency map (for decompression)
    void rebuild(const std::unordered_map<char, int>& freqTable);

    // Returns code map {char -> bit-string like "0", "101", ...}
    const std::unordered_map<char, std::string>& getCodes() const;

    // Root accessor (for decoding)
    const HuffmanNode* getRoot() const;

    // Utility: height of tree
    int height() const;

    // Average code length (weighted by frequency)
    double averageCodeLength() const;

    // Total characters encoded (sum of frequencies)
    int totalCharacters() const;

private:
    HuffmanNode* root_;
    std::unordered_map<char, std::string> codes_;
    std::unordered_map<char, int> freqTable_;   // kept for statistics

    void destroyTree(HuffmanNode* node);
    void generateCodes(HuffmanNode* node, const std::string& prefix);
    int  computeHeight(const HuffmanNode* node) const;
};
