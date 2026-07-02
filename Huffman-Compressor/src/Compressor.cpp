#include "../include/Compressor.h"
#include "../include/HuffmanTree.h"
#include "../include/BitIO.h"

#include <fstream>
#include <sstream>
#include <stdexcept>
#include <chrono>
#include <cstring>   // memcpy

// Magic number that identifies a valid compressed file: 'H','U','F','F'
static constexpr uint32_t MAGIC = 0x46465548u;  // little-endian on disk

// ---------------------------------------------------------------------------
// Helper: write a little-endian uint32_t to a binary stream
// ---------------------------------------------------------------------------
static void writeU32(std::ofstream& out, uint32_t v) {
    out.write(reinterpret_cast<const char*>(&v), 4);
}

// ---------------------------------------------------------------------------
// Helper: write a little-endian uint64_t to a binary stream
// ---------------------------------------------------------------------------
static void writeU64(std::ofstream& out, uint64_t v) {
    out.write(reinterpret_cast<const char*>(&v), 8);
}

// ---------------------------------------------------------------------------
// buildFrequencyTable
// Counts occurrences of every character in the text.
// O(n) where n = text length.
// ---------------------------------------------------------------------------
std::unordered_map<char, int>
Compressor::buildFrequencyTable(const std::string& text) const {
    std::unordered_map<char, int> freq;
    freq.reserve(256);
    for (char c : text)
        ++freq[c];
    return freq;
}

// ---------------------------------------------------------------------------
// compress
//
// Steps:
//   1. Read entire input file into memory.
//   2. Build frequency table.
//   3. Construct Huffman tree and generate codes.
//   4. Write binary file header (magic, size, freq table, bit count, padding).
//   5. Encode every character as Huffman bits via BitWriter.
//   6. Flush partial byte, back-patch the padding count in the header.
//   7. Collect and return statistics.
// ---------------------------------------------------------------------------
CompressionStats Compressor::compress(const std::string& inputPath,
                                       const std::string& outputPath) {
    auto t0 = std::chrono::high_resolution_clock::now();

    // -----------------------------------------------------------------------
    // 1. Read input
    // -----------------------------------------------------------------------
    std::ifstream in(inputPath, std::ios::binary);
    if (!in.is_open())
        throw std::runtime_error("Cannot open input file: " + inputPath);

    std::ostringstream buf;
    buf << in.rdbuf();
    const std::string text = buf.str();
    in.close();

    if (text.empty())
        throw std::runtime_error("Input file is empty — nothing to compress.");

    // -----------------------------------------------------------------------
    // 2. Frequency table
    // -----------------------------------------------------------------------
    auto freqTable = buildFrequencyTable(text);

    // -----------------------------------------------------------------------
    // 3. Build Huffman tree → generate codes
    // -----------------------------------------------------------------------
    HuffmanTree tree;
    tree.build(freqTable);
    const auto& codes = tree.getCodes();

    // -----------------------------------------------------------------------
    // 4. Open output and write header
    //
    // Header layout:
    //   [4]  magic number
    //   [8]  original file size
    //   [4]  number of unique characters
    //   [n*5] frequency table (1 byte char + 4 byte freq) per entry
    //   [8]  encoded bit count  (back-patched after encoding)
    //   [1]  padding bits       (back-patched after encoding)
    // -----------------------------------------------------------------------
    std::ofstream out(outputPath, std::ios::binary);
    if (!out.is_open())
        throw std::runtime_error("Cannot open output file: " + outputPath);

    writeU32(out, MAGIC);
    writeU64(out, static_cast<uint64_t>(text.size()));
    writeU32(out, static_cast<uint32_t>(freqTable.size()));

    for (const auto& [ch, freq] : freqTable) {
        out.put(ch);
        writeU32(out, static_cast<uint32_t>(freq));
    }

    // Reserve space for bit count (8 bytes) + padding (1 byte)
    // We will seek back and fill these in after encoding.
    std::streampos bitCountPos = out.tellp();
    writeU64(out, 0ULL);   // placeholder: encoded bit count
    out.put(0);             // placeholder: padding bits

    // -----------------------------------------------------------------------
    // 5. Encode payload
    // -----------------------------------------------------------------------
    {
        BitWriter writer(out);

        for (char c : text) {
            const std::string& code = codes.at(c);
            for (char bit : code)
                writer.writeBit(bit - '0');
        }

        uint64_t encodedBits = writer.bitsWritten();
        int      padding     = writer.flush();

        // Back-patch header fields
        out.seekp(bitCountPos);
        writeU64(out, encodedBits);
        out.put(static_cast<char>(padding));
    }

    out.close();

    // -----------------------------------------------------------------------
    // 6. Gather statistics
    // -----------------------------------------------------------------------
    auto t1 = std::chrono::high_resolution_clock::now();
    double ms = std::chrono::duration<double, std::milli>(t1 - t0).count();

    // Compressed file size on disk
    std::ifstream sizer(outputPath, std::ios::binary | std::ios::ate);
    uint64_t compressedBytes = static_cast<uint64_t>(sizer.tellg());

    CompressionStats stats;
    stats.originalBytes    = text.size();
    stats.compressedBytes  = compressedBytes;
    stats.uniqueSymbols    = static_cast<int>(freqTable.size());
    stats.treeHeight       = tree.height();
    stats.avgCodeLength    = tree.averageCodeLength();
    stats.compressionRatio = static_cast<double>(text.size()) / compressedBytes;
    stats.spaceSavedPct    = (1.0 - static_cast<double>(compressedBytes) /
                                    static_cast<double>(text.size())) * 100.0;
    stats.elapsedMs        = ms;

    return stats;
}
