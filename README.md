# Pufferfish 🐡

A modular, cross-platform command-line file compression utility powered by a custom implementation of **Huffman Coding**.

This project was built as a Discrete Mathematics final project to demonstrate the practical application of Information Theory and algorithmic concepts.

## 🎓 Mathematical Concepts Demonstrated

- **Huffman Trees** — Binary trees constructed using a greedy algorithm.
- **Priority Queues (Min-Heaps)** — Used for efficient tree construction ($O(N \log N)$ complexity).
- **Greedy Algorithms** — Always merging the two lowest-frequency nodes to build an optimal tree.
- **Prefix-Free Codes** — Variable-length binary codes where no code is a prefix of another, ensuring unambiguous decoding.
- **Information Encoding** — Replacing fixed-width 8-bit characters with variable-length bit sequences.
- **Lossless Compression** — The mathematically guaranteed property that the decoded output is perfectly identical to the original input.
- **Shannon Entropy** — Calculating the theoretical minimum bits per symbol ($H = -\sum p(x) \log_2 p(x)$) from Claude Shannon's Information Theory.

---

## 🛠️ Prerequisites

To compile this project, you need:
- **CMake** (version 3.16 or higher)
- A **C++20** compatible compiler:
  - **Linux/macOS:** GCC 10+ or Clang 10+
  - **Windows:** MSVC (Visual Studio 2019 v16.8 or higher) or MinGW-w64

---

## 🏗️ Building the Project

Pufferfish uses standard CMake to support multiple platforms. The build process varies slightly depending on your operating system.

### 🐧 Linux & macOS

Open your terminal and run:

```bash
# 1. Generate the build files
cmake -B build

# 2. Compile the project
cmake --build build

# 3. Verify it works
./build/puff help
```

### 🪟 Windows (Visual Studio / MSVC)

Open **Developer Command Prompt for VS** or PowerShell and run:

```powershell
# 1. Generate the build files
cmake -B build

# 2. Compile the project (Release mode is recommended for performance)
cmake --build build --config Release

# 3. Verify it works
.\build\Release\puff.exe help
```

> **Note for Windows Users:** Unlike Linux which places the executable directly in the `build/` folder, CMake on Windows (using MSVC generators) typically places the executable inside a configuration subfolder like `build\Release\` or `build\Debug\`.

---

## 🚀 Usage Guide

Pufferfish provides three core commands: `compress`, `extract`, and `analyze`.

### 1. Compress a File
Compresses a target file into a custom `.puff` archive.

* **Linux:** `./build/puff compress samples/sample.txt`
* **Windows:** `.\build\Release\puff.exe compress samples\sample.txt`

You can also use the shorthand `c`:
```bash
./build/puff c novel.txt
```
*(Produces `novel.puff` in the same directory).*

### 2. Extract a File
Restores a `.puff` archive back to its original file name and content.

* **Linux:** `./build/puff extract samples/sample.puff`
* **Windows:** `.\build\Release\puff.exe extract samples\sample.puff`

You can also use the shorthand `x`:
```bash
./build/puff x novel.puff
```

### 3. Analyze a File (Educational Feature)
The `analyze` command is the centerpiece of the educational aspect of this project. It simulates the Huffman compression process in memory and outputs a detailed mathematical analysis without creating a file.

* **Linux:** `./build/puff analyze samples/sample.txt`
* **Windows:** `.\build\Release\puff.exe analyze samples\sample.txt`

**Sample Output:**
```text
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
  Top Symbols:
    ' '    →  13.2%   Code: 101
    'e'    →  10.0%   Code: 000
    'a'    →   6.1%   Code: 1000
    's'    →   6.0%   Code: 0111
    't'    →   5.8%   Code: 0110
═══════════════════════════════════════════════════
```

---

## 📚 Documentation

For a deeper dive into the computer science and math behind the project:

- [docs/algorithm.md](docs/algorithm.md) — A step-by-step walkthrough of how Huffman Coding works, including a manual trace of building a tree and calculating entropy.
- [docs/archive_format.md](docs/archive_format.md) — The byte-level specification of the custom `.puff` binary archive format.

---

## 📂 Project Structure

The codebase is organized into 4 logical modules to strictly separate the computer science algorithms from file system I/O and user interfaces.

```text
pufferfish/
├── CMakeLists.txt              # Cross-platform build configuration
├── include/
│   ├── huffman.hpp             # Module 1: Core math, trees, and logic
│   ├── archive.hpp             # Module 2: .puff format reader/writer
│   └── statistics.hpp          # Module 3: Analysis and entropy calculations
└── src/
    ├── huffman.cpp             
    ├── archive.cpp             
    ├── statistics.cpp          
    └── main.cpp                # Module 4: CLI application entry point
```

## 📜 License
This project is open-source and intended for educational purposes.
