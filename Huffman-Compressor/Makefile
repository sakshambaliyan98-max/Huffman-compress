CXX      := g++
CXXFLAGS := -std=c++17 -Wall -Wextra -Wpedantic -O2

# Directories
SRCDIR   := src
INCDIR   := include
OBJDIR   := obj
BINDIR   := .

# Sources & objects
SOURCES  := $(wildcard $(SRCDIR)/*.cpp)
OBJECTS  := $(patsubst $(SRCDIR)/%.cpp,$(OBJDIR)/%.o,$(SOURCES))

TARGET   := huffman

.PHONY: all clean run-test

all: $(OBJDIR) $(TARGET)

$(OBJDIR):
	mkdir -p $(OBJDIR)

$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) -o $@ $^
	@echo "Build successful: ./$(TARGET)"

$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -I$(INCDIR) -c $< -o $@

# Quick sanity test using the bundled sample
run-test: $(TARGET)
	@echo "--- Compressing sample.txt ---"
	./$(TARGET) compress input/sample.txt output/compressed.bin
	@echo ""
	@echo "--- Decompressing compressed.bin ---"
	./$(TARGET) decompress output/compressed.bin output/decoded.txt
	@echo ""
	@echo "--- Verifying round-trip ---"
	diff input/sample.txt output/decoded.txt && echo "PASS: files are identical" || echo "FAIL: files differ"

clean:
	rm -rf $(OBJDIR) $(TARGET) output/compressed.bin output/decoded.txt
