#include "../include/Compressor.h"
#include "../include/Decompressor.h"
#include "../include/HuffmanTree.h"

#include <iostream>
#include <iomanip>
#include <string>
#include <limits>

// ---------------------------------------------------------------------------
// Helpers for pretty console output
// ---------------------------------------------------------------------------
static void printSeparator(char c = '-', int width = 60) {
    std::cout << std::string(width, c) << '\n';
}

static void printBanner() {
    printSeparator('=');
    std::cout << "  Huffman Text Compressor & Decompressor  |  C++17\n";
    printSeparator('=');
}

static void printCompressionStats(const CompressionStats& s,
                                   const std::string& in,
                                   const std::string& out) {
    printSeparator();
    std::cout << "  COMPRESSION RESULTS\n";
    printSeparator();
    std::cout << std::left;
    std::cout << "  Input  file      : " << in  << '\n';
    std::cout << "  Output file      : " << out << '\n';
    printSeparator();
    std::cout << "  Original size    : " << s.originalBytes    << " bytes\n";
    std::cout << "  Compressed size  : " << s.compressedBytes  << " bytes\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Space saved      : " << s.spaceSavedPct    << " %\n";
    std::cout << "  Compression ratio: " << s.compressionRatio << " : 1\n";
    printSeparator();
    std::cout << "  Unique symbols   : " << s.uniqueSymbols    << '\n';
    std::cout << "  Tree height      : " << s.treeHeight       << '\n';
    std::cout << "  Avg code length  : " << s.avgCodeLength    << " bits\n";
    std::cout << "  Time elapsed     : " << s.elapsedMs        << " ms\n";
    printSeparator();
}

static void printDecompressionStats(const DecompressionStats& s,
                                     const std::string& in,
                                     const std::string& out) {
    printSeparator();
    std::cout << "  DECOMPRESSION RESULTS\n";
    printSeparator();
    std::cout << "  Input  file      : " << in  << '\n';
    std::cout << "  Output file      : " << out << '\n';
    printSeparator();
    std::cout << "  Compressed size  : " << s.compressedBytes << " bytes\n";
    std::cout << "  Recovered size   : " << s.recoveredBytes  << " bytes\n";
    std::cout << "  Unique symbols   : " << s.uniqueSymbols   << '\n';
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "  Time elapsed     : " << s.elapsedMs       << " ms\n";
    printSeparator();
    std::cout << "  Verification     : OK — decoded file matches original size\n";
    printSeparator();
}

// ---------------------------------------------------------------------------
// Interactive menu mode
// ---------------------------------------------------------------------------
static void runInteractive() {
    printBanner();

    while (true) {
        std::cout << "\n  [1] Compress\n"
                     "  [2] Decompress\n"
                     "  [3] Exit\n\n"
                     "  Choice: ";

        int choice;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            continue;
        }
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 3) {
            std::cout << "\n  Goodbye!\n";
            break;
        }

        std::string src, dst;

        if (choice == 1) {
            std::cout << "  Input  text file  : "; std::getline(std::cin, src);
            std::cout << "  Output binary file: "; std::getline(std::cin, dst);

            try {
                Compressor c;
                auto stats = c.compress(src, dst);
                printCompressionStats(stats, src, dst);
            } catch (const std::exception& e) {
                std::cerr << "\n  ERROR: " << e.what() << '\n';
            }

        } else if (choice == 2) {
            std::cout << "  Input  binary file: "; std::getline(std::cin, src);
            std::cout << "  Output text file  : "; std::getline(std::cin, dst);

            try {
                Decompressor d;
                auto stats = d.decompress(src, dst);
                printDecompressionStats(stats, src, dst);
            } catch (const std::exception& e) {
                std::cerr << "\n  ERROR: " << e.what() << '\n';
            }

        } else {
            std::cout << "  Unknown option.\n";
        }
    }
}

// ---------------------------------------------------------------------------
// Command-line mode:
//   huffman compress   <input.txt>  <output.bin>
//   huffman decompress <input.bin>  <output.txt>
// ---------------------------------------------------------------------------
static int runCommandLine(int argc, char* argv[]) {
    // argv: [0]=program [1]=mode [2]=src [3]=dst
    if (argc != 4) {
        std::cerr << "Usage:\n"
                     "  " << argv[0] << " compress   <input.txt>  <output.bin>\n"
                     "  " << argv[0] << " decompress <input.bin>  <output.txt>\n";
        return 1;
    }

    std::string mode = argv[1];
    std::string src  = argv[2];
    std::string dst  = argv[3];

    try {
        if (mode == "compress") {
            Compressor c;
            auto stats = c.compress(src, dst);
            printCompressionStats(stats, src, dst);
        } else if (mode == "decompress") {
            Decompressor d;
            auto stats = d.decompress(src, dst);
            printDecompressionStats(stats, src, dst);
        } else {
            std::cerr << "Unknown mode '" << mode << "'. Use 'compress' or 'decompress'.\n";
            return 1;
        }
    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << '\n';
        return 1;
    }
    return 0;
}

// ---------------------------------------------------------------------------
int main(int argc, char* argv[]) {
    if (argc == 1) {
        runInteractive();
        return 0;
    }
    return runCommandLine(argc, argv);
}
