#pragma once

#include <string>
#include <unordered_map>
#include <cstdint>

// ---------------------------------------------------------------------------
// CompressionStats — returned to caller after compress()
// ---------------------------------------------------------------------------
struct CompressionStats {
    uint64_t originalBytes    = 0;
    uint64_t compressedBytes  = 0;   // size of .bin file on disk
    uint64_t encodedBits      = 0;   // pure payload bits
    int      uniqueSymbols    = 0;
    int      treeHeight       = 0;
    double   avgCodeLength    = 0.0;
    double   compressionRatio = 0.0; // originalBytes / compressedBytes
    double   spaceSavedPct    = 0.0; // (1 - ratio) * 100
    double   elapsedMs        = 0.0;
};

// ---------------------------------------------------------------------------
// Compressor
// Reads a text file, builds a Huffman tree, encodes the content, and
// writes a structured binary compressed file.
//
// Binary file format (all multi-byte integers in little-endian):
//
//   Bytes   Field
//   ------  --------------------------------------------------------
//   4       Magic number  0x48554646 ('H','U','F','F')
//   8       Original file size (uint64_t)
//   4       Number of unique characters (uint32_t)
//   N*5     Frequency table entries: 1 byte char + 4 bytes uint32_t freq
//   8       Encoded bit count (uint64_t)  — excludes padding
//   1       Padding bits in the last byte (uint8_t, 0-7)
//   ...     Compressed binary payload
// ---------------------------------------------------------------------------
class Compressor {
public:
    Compressor() = default;

    // Compress inputPath -> outputPath.  Returns stats.
    // Throws std::runtime_error on any I/O or logical error.
    CompressionStats compress(const std::string& inputPath,
                              const std::string& outputPath);

private:
    std::unordered_map<char, int> buildFrequencyTable(const std::string& text) const;
};
