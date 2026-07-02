#include "../include/Decompressor.h"
#include "../include/HuffmanTree.h"
#include "../include/BitIO.h"

#include <fstream>
#include <stdexcept>
#include <chrono>
#include <unordered_map>

static constexpr uint32_t MAGIC = 0x46465548u;

static uint32_t readU32(std::ifstream& in) {
    uint32_t v = 0;
    in.read(reinterpret_cast<char*>(&v), 4);
    return v;
}

static uint64_t readU64(std::ifstream& in) {
    uint64_t v = 0;
    in.read(reinterpret_cast<char*>(&v), 8);
    return v;
}

// ---------------------------------------------------------------------------
// decompress
//
// Decodes until exactly originalSize characters have been recovered.
// Stopping on character count (not bit count) means we never
// accidentally parse the zero-padding bits at the tail of the last byte
// as an extra symbol.
// ---------------------------------------------------------------------------
DecompressionStats Decompressor::decompress(const std::string& inputPath,
                                             const std::string& outputPath) {
    auto t0 = std::chrono::high_resolution_clock::now();

    std::ifstream in(inputPath, std::ios::binary);
    if (!in.is_open())
        throw std::runtime_error("Cannot open compressed file: " + inputPath);

    // ── Header ───────────────────────────────────────────────────────────────
    uint32_t magic = readU32(in);
    if (magic != MAGIC)
        throw std::runtime_error(
            "Invalid file format: magic number mismatch. "
            "Is this a valid .huff file?");

    uint64_t originalSize = readU64(in);
    uint32_t uniqueChars  = readU32(in);

    if (uniqueChars == 0)
        throw std::runtime_error("Corrupt header: zero unique characters.");

    std::unordered_map<char, int> freqTable;
    freqTable.reserve(uniqueChars);
    for (uint32_t i = 0; i < uniqueChars; ++i) {
        char     ch   = static_cast<char>(in.get());
        uint32_t freq = readU32(in);
        freqTable[ch] = static_cast<int>(freq);
    }

    // Read (and discard) bit-count + padding — stored for forward-compat
    (void)readU64(in);          // encoded bit count
    (void)in.get();             // padding bits

    // ── Reconstruct Huffman tree ─────────────────────────────────────────────
    HuffmanTree tree;
    tree.rebuild(freqTable);
    const HuffmanNode* root = tree.getRoot();
    if (!root)
        throw std::runtime_error("Failed to reconstruct Huffman tree.");

    // ── Decode payload ───────────────────────────────────────────────────────
    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open())
        throw std::runtime_error("Cannot open output file: " + outputPath);

    BitReader reader(in);

    uint64_t charsDecoded = 0;
    bool     singleChar   = root->isLeaf();
    const HuffmanNode* current = root;

    // Decode until we have the exact original number of characters.
    // This is correct because the Huffman code is prefix-free: the tree
    // walk always lands on a leaf at the end of a valid codeword, so we
    // never read one bit too many.  The padding bits at the tail of the
    // last byte are never consumed.
    while (charsDecoded < originalSize) {
        int bit = reader.readBit();

        if (singleChar) {
            out.put(root->ch);
            ++charsDecoded;
            continue;
        }

        current = (bit == 0) ? current->left : current->right;

        if (!current)
            throw std::runtime_error(
                "Corrupt compressed data: null tree node reached.");

        if (current->isLeaf()) {
            out.put(current->ch);
            ++charsDecoded;
            current = root;
        }
    }

    in.close();
    out.close();

    // ── Stats ────────────────────────────────────────────────────────────────
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    std::ifstream sizer(inputPath, std::ios::binary | std::ios::ate);
    uint64_t compressedBytes = static_cast<uint64_t>(sizer.tellg());

    DecompressionStats stats;
    stats.compressedBytes  = compressedBytes;
    stats.recoveredBytes   = charsDecoded;
    stats.uniqueSymbols    = static_cast<int>(freqTable.size());
    stats.elapsedMs        = ms;
    return stats;
}
