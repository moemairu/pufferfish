<div align="center">
  
# 🐡 Pufferfish

**A lightweight, cross-platform command-line file compression utility powered by a custom implementation of Huffman Coding.**

[![C++20](https://img.shields.io/badge/C++-20-blue.svg?style=flat&logo=c%2B%2B)](https://en.cppreference.com/w/cpp/20)
[![CMake](https://img.shields.io/badge/CMake-3.16+-brightgreen.svg?style=flat&logo=cmake)](https://cmake.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

*Built as a Discrete Mathematics final project to demonstrate the practical application of Information Theory and algorithmic concepts.*

[Features](#-features) • [Mathematical Concepts](#-mathematical-concepts) • [Installation](#%EF%B8%8F-installation) • [Usage](#-usage-guide) • [Architecture](#%EF%B8%8F-architecture)
</div>

---

## ✨ Features

- **Blazing Fast**: Written in pure C++20 with raw performance in mind.
- **Cross-Platform**: Compiles seamlessly on Linux, macOS, and Windows.
- **Deep Mathematical Analysis**: Provides theoretical entropy and compression efficiency reports.
- **No Dependencies**: Relies solely on the C++ Standard Library.
- **Zero Abstraction Bloat**: Clean, highly modular architecture spanning only 4 core modules.

---

## 🎓 Mathematical Concepts

This project acts as an interactive proof for core concepts in **Discrete Mathematics** and **Information Theory**:

| Concept | Demonstration |
| :--- | :--- |
| **Huffman Trees** | Binary trees constructed using a deterministic greedy algorithm. |
| **Priority Queues** | $\mathcal{O}(N \log N)$ complexity tree construction using Min-Heaps. |
| **Prefix-Free Codes** | Variable-length binary codes where no code is a prefix of another, ensuring unambiguous decoding. |
| **Shannon Entropy** | Calculates the theoretical minimum bits per symbol ($H = -\sum p(x) \log_2 p(x)$). |
| **Lossless Compression** | Mathematical guarantee that decoded output is perfectly identical to the original input. |

---

## 🛠️ Installation

### Prerequisites
- **CMake** (v3.16+)
- **C++20 Compiler**: GCC 10+, Clang 10+, or MSVC v16.8+

### 🐧 Linux & macOS
```bash
# 1. Generate the build files
cmake -B build

# 2. Compile the project
cmake --build build

# 3. Verify it works
./build/puff help
```

### 🪟 Windows (MSVC)
```powershell
# 1. Generate the build files
cmake -B build

# 2. Compile the project (Release mode is recommended)
cmake --build build --config Release

# 3. Verify it works
.\build\Release\puff.exe help
```

---

## 🚀 Usage Guide

Pufferfish uses a simple and intuitive CLI. 

### 📦 Compress a File
Compresses a target file into a custom `.puff` archive.
```bash
./build/puff compress samples/sample.txt

# Or use shorthand:
./build/puff c samples/sample.txt
```

### 🔓 Extract a File
Restores a `.puff` archive back to its original state.
```bash
./build/puff extract samples/sample.puff

# Or use shorthand:
./build/puff x samples/sample.puff
```

### 📊 Analyze a File *(Educational Feature)*
Simulates the Huffman compression process in memory and outputs a detailed mathematical analysis **without** creating a file.
```bash
./build/puff analyze samples/sample.txt
```

<details>
<summary><b>View Sample Output</b></summary>

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
</details>

---

## 🏗️ Architecture

The codebase is organized into **4 logical modules** to strictly separate computer science algorithms from file system I/O and user interfaces.

```text
pufferfish/
├── CMakeLists.txt              # Cross-platform build configuration
├── include/                    
│   ├── huffman.hpp             # 🧠 Core math, trees, and logic
│   ├── archive.hpp             # 💾 .puff format reader/writer
│   └── statistics.hpp          # 📈 Analysis and entropy calculations
└── src/
    ├── huffman.cpp             
    ├── archive.cpp             
    ├── statistics.cpp          
    └── main.cpp                # 🖥️ CLI application entry point
```

---

## 📚 Further Reading

For a deeper dive into the math and architecture behind the project:
- [**The Algorithm**](docs/algorithm.md): Step-by-step walkthrough of Huffman Coding, including a manual trace of building a tree and calculating entropy.
- [**The `.puff` Format**](docs/archive_format.md): Byte-level specification of the custom binary archive.

---

<div align="center">
  <i>Open-source and built for educational purposes.</i>
</div>
