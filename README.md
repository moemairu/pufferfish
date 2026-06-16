# Pufferfish

A modular, command-line file compression utility powered by a custom implementation of **Huffman Coding**.

Built as a Discrete Mathematics final project demonstrating:

- **Huffman Trees** — binary trees constructed using a greedy algorithm
- **Priority Queues (Min-Heaps)** — for efficient tree construction
- **Greedy Algorithms** — always merging the two lowest-frequency nodes
- **Prefix Codes** — variable-length codes where no code is a prefix of another
- **Information Encoding** — replacing fixed-width bytes with variable-length bit sequences
- **Lossless Compression** — decoded output is identical to the original input
- **Shannon Entropy** — the theoretical minimum bits per symbol from Information Theory

## Building

The project uses CMake and follows a clean separation of concerns.

```bash
cmake -B build
cmake --build build
```

Requires **C++20** (minimum C++17). Tested on Linux and Windows.

## Usage

### Compress a file

```bash
./build/puff compress samples/sample.txt
```

Produces `samples/sample.puff`.

### Extract a file

```bash
./build/puff extract samples/sample.puff
```

Restores `samples/sample.txt`.

### Analyze a file

```bash
./build/puff analyze samples/sample.txt
```

Displays detailed Huffman statistics:

```
═══════════════════════════════════════════════════
  Pufferfish — Huffman Analysis
═══════════════════════════════════════════════════
  File:                    sample.txt
  Total Symbols:           1,496
  Unique Symbols:          70
───────────────────────────────────────────────────
  Tree Height:             13
  Total Nodes:             139
───────────────────────────────────────────────────
  Shannon Entropy:         4.65 bits/symbol
  Average Code Length:     4.67 bits/symbol
  Compression Efficiency:  99.5%
───────────────────────────────────────────────────
...
```

### Shorthand

```bash
./build/puff c samples/sample.txt      # compress
./build/puff x samples/sample.puff     # extract
./build/puff a samples/sample.txt      # analyze
```

## How Huffman Coding Works

1. **Frequency Analysis** — count how often each byte appears in the input
2. **Tree Construction** — build a binary tree using a min-heap, always merging the two least frequent nodes (greedy choice)
3. **Code Generation** — traverse the tree: left = 0, right = 1; each leaf gets a unique prefix-free code
4. **Encoding** — replace each input byte with its variable-length code
5. **Decoding** — traverse the tree bit-by-bit to recover the original bytes

See [docs/algorithm.md](docs/algorithm.md) for a detailed walkthrough.

## .puff Archive Format

See [docs/archive_format.md](docs/archive_format.md) for the byte-level specification.

## Project Structure

The project is broken down into 4 core modules for clean abstraction:

```text
pufferfish/
├── CMakeLists.txt              # Build configuration
├── include/
│   ├── huffman.hpp             # Core math, trees, and logic
│   ├── archive.hpp             # .puff format reader/writer
│   └── statistics.hpp          # Analysis calculations
└── src/
    ├── huffman.cpp             
    ├── archive.cpp             
    ├── statistics.cpp          
    └── main.cpp                # CLI application
```
