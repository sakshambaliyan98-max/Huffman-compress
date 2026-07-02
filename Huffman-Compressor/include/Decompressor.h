#pragma once

#include <string>
#include <cstdint>

// ---------------------------------------------------------------------------
// DecompressionStats — returned to caller after decompress()
// ---------------------------------------------------------------------------
struct DecompressionStats {
    uint64_t compressedBytes  = 0;
    uint64_t recoveredBytes   = 0;
    int      uniqueSymbols    = 0;
    double   elapsedMs        = 0.0;
};

// ---------------------------------------------------------------------------
// Decompressor
// Reads a compressed binary file produced by Compressor, reconstructs the
// Huffman tree from the embedded frequency table, and decodes the payload
// bit-by-bit to recover the exact original text.
// ---------------------------------------------------------------------------
class Decompressor {
public:
    Decompressor() = default;

    // Decompress inputPath -> outputPath.  Returns stats.
    // Throws std::runtime_error on format errors or I/O failures.
    DecompressionStats decompress(const std::string& inputPath,
                                  const std::string& outputPath);
};
