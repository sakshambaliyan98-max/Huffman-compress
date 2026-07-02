#include "../include/BitIO.h"

// ===========================================================================
// BitWriter
// ===========================================================================

BitWriter::BitWriter(std::ofstream& out) : out_(out) {}

// ---------------------------------------------------------------------------
// writeBit — accumulate bits MSB-first into a byte buffer.
// When 8 bits are collected, flush the byte to the stream.
// ---------------------------------------------------------------------------
void BitWriter::writeBit(int bit) {
    // Shift buffer left and OR in the new bit
    buffer_ = static_cast<uint8_t>((buffer_ << 1) | (bit & 1));
    ++bitCount_;
    ++bitsWritten_;

    if (bitCount_ == 8) {
        out_.put(static_cast<char>(buffer_));
        buffer_   = 0;
        bitCount_ = 0;
    }
}

// ---------------------------------------------------------------------------
// writeByte — convenience: write all 8 bits of a byte
// ---------------------------------------------------------------------------
void BitWriter::writeByte(uint8_t byte) {
    for (int i = 7; i >= 0; --i)
        writeBit((byte >> i) & 1);
}

// ---------------------------------------------------------------------------
// flush — pad the partial byte with 0s and write it.
// Returns the number of padding bits (0 if buffer was already empty).
// ---------------------------------------------------------------------------
int BitWriter::flush() {
    if (bitCount_ == 0) return 0;

    int padding = 8 - bitCount_;
    buffer_ = static_cast<uint8_t>(buffer_ << padding);
    out_.put(static_cast<char>(buffer_));
    buffer_   = 0;
    bitCount_ = 0;
    return padding;
}

// ===========================================================================
// BitReader
// ===========================================================================

BitReader::BitReader(std::ifstream& in) : in_(in) {}

// ---------------------------------------------------------------------------
// readBit — return the next bit from the buffered byte.
// When the buffer is exhausted, read the next byte from the stream.
// ---------------------------------------------------------------------------
int BitReader::readBit() {
    if (bitCount_ == 0) {
        int c = in_.get();
        if (c == EOF) {
            eof_ = true;
            throw std::runtime_error("BitReader: unexpected end of compressed data.");
        }
        buffer_   = static_cast<uint8_t>(c);
        bitCount_ = 8;
    }
    // Return the MSB
    int bit = (buffer_ >> 7) & 1;
    buffer_ = static_cast<uint8_t>(buffer_ << 1);
    --bitCount_;
    return bit;
}

// ---------------------------------------------------------------------------
// readByte — read 8 consecutive bits
// ---------------------------------------------------------------------------
uint8_t BitReader::readByte() {
    uint8_t result = 0;
    for (int i = 0; i < 8; ++i)
        result = static_cast<uint8_t>((result << 1) | readBit());
    return result;
}

bool BitReader::eof() const { return eof_; }
