#pragma once

#include <fstream>
#include <cstdint>
#include <stdexcept>

// ---------------------------------------------------------------------------
// BitWriter
// Packs individual bits into bytes and writes them to a binary output stream.
// Must call flush() before closing the stream to write any partial byte.
// ---------------------------------------------------------------------------
class BitWriter {
public:
    explicit BitWriter(std::ofstream& out);

    // Write a single bit (0 or 1)
    void writeBit(int bit);

    // Write all 8 bits of a full byte (convenience)
    void writeByte(uint8_t byte);

    // Flush remaining bits (pads with 0s to complete the last byte).
    // Returns the number of padding bits added (0-7).
    int flush();

    // How many bits have been written so far (not counting padding)
    uint64_t bitsWritten() const { return bitsWritten_; }

private:
    std::ofstream& out_;
    uint8_t        buffer_     = 0;
    int            bitCount_   = 0;   // bits accumulated in buffer_
    uint64_t       bitsWritten_= 0;
};

// ---------------------------------------------------------------------------
// BitReader
// Reads individual bits from a binary input stream.
// ---------------------------------------------------------------------------
class BitReader {
public:
    explicit BitReader(std::ifstream& in);

    // Read a single bit. Returns 0 or 1.
    // Throws std::runtime_error if no more bytes available.
    int readBit();

    // Read a full byte (8 bits).
    uint8_t readByte();

    bool eof() const;

private:
    std::ifstream& in_;
    uint8_t        buffer_   = 0;
    int            bitCount_ = 0;   // valid bits remaining in buffer_
    bool           eof_      = false;
};
