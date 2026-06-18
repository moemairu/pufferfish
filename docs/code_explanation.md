# Penjelasan Kode Pufferfish — Lengkap & Detail

Dokumen ini menjelaskan **keseluruhan kode** proyek Pufferfish secara mendetail, mulai dari arsitektur, alur kerja (flow), hingga penjelasan sintaks C++ baris per baris.

---

## Daftar Isi

1. [Arsitektur & Struktur Proyek](#1-arsitektur--struktur-proyek)
2. [Sistem Build — CMakeLists.txt](#2-sistem-build--cmakeliststxt)
3. [Alur Kerja Keseluruhan (Flow)](#3-alur-kerja-keseluruhan-flow)
4. [Module 1: Huffman Core — huffman.hpp & huffman.cpp](#4-module-1-huffman-core)
   - 4.1 [Type Aliases](#41-type-aliases)
   - 4.2 [HuffmanNode (Struktur Data Node)](#42-huffmannode)
   - 4.3 [HuffmanTree (Konstruksi Pohon)](#43-huffmantree)
   - 4.4 [BitWriter & BitReader (I/O Bit-Level)](#44-bitwriter--bitreader)
   - 4.5 [Encoder & Decoder](#45-encoder--decoder)
5. [Module 2: Archive — archive.hpp & archive.cpp](#5-module-2-archive)
   - 5.1 [Format Biner .puff](#51-format-biner-puff)
   - 5.2 [Helper Fungsi Serialisasi](#52-helper-fungsi-serialisasi)
   - 5.3 [ArchiveWriter::compress()](#53-archivewritercompress)
   - 5.4 [ArchiveReader::extract()](#54-archivereaderextract)
6. [Module 3: Statistics — statistics.hpp & statistics.cpp](#6-module-3-statistics)
   - 6.1 [AnalysisResult (Struktur Data Hasil)](#61-analysisresult)
   - 6.2 [Statistics::analyze()](#62-statisticsanalyze)
   - 6.3 [Statistics::print_report()](#63-statisticsprint_report)
7. [Module 4: CLI Entry Point — main.cpp](#7-module-4-cli-entry-point)
8. [Konsep Matematika dalam Kode](#8-konsep-matematika-dalam-kode)
9. [Glossary Sintaks C++ yang Digunakan](#9-glossary-sintaks-c-yang-digunakan)

---

## 1. Arsitektur & Struktur Proyek

```text
pufferfish/
├── CMakeLists.txt              ← Konfigurasi build (CMake)
├── include/                    ← Header files (deklarasi interface)
│   ├── huffman.hpp             ← Deklarasi Node, Tree, BitIO, Encoder, Decoder
│   ├── archive.hpp             ← Deklarasi ArchiveWriter & ArchiveReader
│   └── statistics.hpp          ← Deklarasi Statistics & AnalysisResult
├── src/                        ← Source files (implementasi)
│   ├── huffman.cpp             ← Implementasi seluruh isi huffman.hpp
│   ├── archive.cpp             ← Implementasi compress & extract
│   ├── statistics.cpp          ← Implementasi analyze & print_report
│   └── main.cpp                ← Entry point CLI (fungsi main)
├── docs/                       ← Dokumentasi
└── samples/                    ← File contoh untuk pengujian
```

### Prinsip Desain

| Prinsip | Penerapan |
| :--- | :--- |
| **Separation of Concerns** | Setiap modul punya tanggung jawab tunggal |
| **Header/Source Split** | `.hpp` berisi deklarasi (apa), `.cpp` berisi implementasi (bagaimana) |
| **Namespace** | Seluruh kode berada dalam `namespace pufferfish` untuk menghindari konflik nama |
| **Include Guard** | Setiap `.hpp` menggunakan `#ifndef`/`#define`/`#endif` agar tidak di-include dua kali |

### Diagram Dependensi Antar Modul

```text
main.cpp
  ├── #include "archive.hpp"     →  archive.cpp
  │       └── #include "huffman.hpp"  →  huffman.cpp
  └── #include "statistics.hpp"  →  statistics.cpp
            └── #include "huffman.hpp"  →  huffman.cpp
```

`huffman.hpp` adalah **fondasi** — semua modul lain bergantung padanya, tapi dia sendiri tidak bergantung pada modul lain.

---

## 2. Sistem Build — CMakeLists.txt

```cmake
cmake_minimum_required(VERSION 3.16)
project(pufferfish VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(include)

add_executable(puff
    src/huffman.cpp
    src/archive.cpp
    src/statistics.cpp
    src/main.cpp
)
```

### Penjelasan Baris per Baris

| Baris | Penjelasan |
| :--- | :--- |
| `cmake_minimum_required(VERSION 3.16)` | Menetapkan versi minimum CMake yang dibutuhkan. Versi 3.16 mendukung C++20. |
| `project(pufferfish VERSION 1.0.0 LANGUAGES CXX)` | Mendefinisikan nama proyek, versi semantik, dan bahasa yang digunakan (C++). |
| `set(CMAKE_CXX_STANDARD 20)` | Menggunakan standar C++20. Diperlukan untuk fitur seperti `std::optional`, structured bindings, dan `[[nodiscard]]`. |
| `set(CMAKE_CXX_STANDARD_REQUIRED ON)` | Jika compiler tidak mendukung C++20, build akan **gagal** (bukan fallback ke versi lama). |
| `include_directories(include)` | Menambahkan folder `include/` ke search path, sehingga `#include "huffman.hpp"` bisa ditemukan oleh compiler. |
| `add_executable(puff ...)` | Mendefinisikan target executable bernama `puff` dari 4 file `.cpp`. CMake akan mengompilasi setiap `.cpp` menjadi *object file* lalu *link* menjadi satu executable. |

---

## 3. Alur Kerja Keseluruhan (Flow)

### 3.1 Flow Compress

```text
┌─────────────────────────────────────────────────────────────┐
│  ./puff compress samples/sample.txt                         │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  main.cpp              │
              │  Parsing argument CLI  │
              │  cmd = "compress"      │
              └───────────┬────────────┘
                          │
                          ▼
              ┌────────────────────────┐
              │  ArchiveWriter::       │
              │  compress(file_path)   │
              │  (archive.cpp)         │
              └───────────┬────────────┘
                          │
          ┌───────────────┼───────────────────┐
          ▼               ▼                   ▼
  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐
  │ Baca file    │  │ Hitung       │  │ Bangun Huffman   │
  │ ke memory    │  │ frekuensi    │  │ Tree (greedy)    │
  │ (ifstream)   │  │ tiap byte    │  │ dari min-heap    │
  └──────┬───────┘  └──────┬───────┘  └────────┬─────────┘
         │                 │                    │
         │                 │                    ▼
         │                 │          ┌──────────────────┐
         │                 │          │ Generate kode    │
         │                 │          │ prefix-free      │
         │                 │          │ (traversal DFS)  │
         │                 │          └────────┬─────────┘
         │                 │                   │
         ▼                 ▼                   ▼
  ┌──────────────────────────────────────────────────┐
  │  Encoder::encode()                               │
  │  Ganti tiap byte → kode Huffman (bit-by-bit)     │
  │  via BitWriter                                   │
  └──────────────────────┬───────────────────────────┘
                         │
                         ▼
  ┌──────────────────────────────────────────────────┐
  │  Tulis file .puff:                               │
  │  [MAGIC][VER][NAMA][SIZE][FREQ TABLE][BITS]      │
  └──────────────────────────────────────────────────┘
```

### 3.2 Flow Extract

```text
┌─────────────────────────────────────────────────────────────┐
│  ./puff extract samples/sample.puff                         │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  ArchiveReader::       │
              │  extract(archive_path) │
              └───────────┬────────────┘
                          │
          ┌───────────────┼───────────────────┐
          ▼               ▼                   ▼
  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐
  │ Baca header  │  │ Baca tabel   │  │ Rebuild Huffman  │
  │ magic, ver,  │  │ frekuensi    │  │ Tree (identik    │
  │ nama, size   │  │ dari header  │  │ dengan compress) │
  └──────────────┘  └──────┬───────┘  └────────┬─────────┘
                           │                    │
                           │                    ▼
                           │          ┌──────────────────┐
                           │          │ Decoder::decode() │
                           │          │ Traversal pohon  │
                           │          │ bit-by-bit via   │
                           │          │ BitReader        │
                           │          └────────┬─────────┘
                           │                   │
                           ▼                   ▼
                    ┌──────────────────────────────────┐
                    │  Tulis file asli (byte-by-byte)  │
                    │  Lossless — identik 100%         │
                    └──────────────────────────────────┘
```

### 3.3 Flow Analyze

```text
┌─────────────────────────────────────────────────────────────┐
│  ./puff analyze samples/sample.txt                          │
└──────────────────────────┬──────────────────────────────────┘
                           │
                           ▼
              ┌────────────────────────┐
              │  Statistics::analyze() │
              │  (statistics.cpp)      │
              └───────────┬────────────┘
                          │
          ┌───────────────┼───────────────────┐
          ▼               ▼                   ▼
  ┌──────────────┐  ┌──────────────┐  ┌──────────────────┐
  │ Hitung       │  │ Bangun tree  │  │ Hitung:          │
  │ frekuensi    │  │ + generate   │  │ • Shannon        │
  │              │  │ codes        │  │   Entropy        │
  │              │  │              │  │ • Avg Code Len   │
  │              │  │              │  │ • Efficiency      │
  └──────────────┘  └──────────────┘  └──────────────────┘
                          │
                          ▼
              ┌────────────────────────┐
              │  Statistics::          │
              │  print_report()        │
              │  Tampilkan tabel ke    │
              │  stdout                │
              └────────────────────────┘
```

---

## 4. Module 1: Huffman Core

File: `include/huffman.hpp` (deklarasi) + `src/huffman.cpp` (implementasi)

Modul ini berisi **seluruh inti algoritma Huffman Coding**. Ini adalah jantung proyek.

---

### 4.1 Type Aliases

```cpp
using ByteFrequencyMap = std::unordered_map<uint8_t, uint64_t>;
using HuffmanCodeMap   = std::unordered_map<uint8_t, std::vector<bool>>;
```

| Alias | Tipe Asli | Kegunaan |
| :--- | :--- | :--- |
| `ByteFrequencyMap` | `unordered_map<uint8_t, uint64_t>` | Memetakan setiap byte (0–255) ke berapa kali dia muncul dalam file. Contoh: `{'a' → 50, 'b' → 12}` |
| `HuffmanCodeMap` | `unordered_map<uint8_t, vector<bool>>` | Memetakan setiap byte ke kode Huffman-nya (deretan bit). Contoh: `{'a' → [1,0,1], 'b' → [0,0,1,1]}` |

**Mengapa `uint8_t`?** Karena satu byte memiliki 256 kemungkinan nilai (0–255). Tipe `uint8_t` mewakili ini secara tepat.

**Mengapa `vector<bool>`?** Karena kode Huffman memiliki panjang variabel (bukan kelipatan 8). `vector<bool>` memungkinkan kita menyimpan urutan bit dengan panjang berapa pun.

**Mengapa `unordered_map`?** Karena pencarian $O(1)$ rata-rata, dibandingkan `std::map` yang $O(\log n)$. Untuk tabel frekuensi yang sering diakses, ini lebih efisien.

---

### 4.2 HuffmanNode

#### Deklarasi (huffman.hpp)

```cpp
struct HuffmanNode {
    uint8_t  symbol    = 0;
    uint64_t frequency = 0;
    std::unique_ptr<HuffmanNode> left;
    std::unique_ptr<HuffmanNode> right;

    HuffmanNode(uint8_t sym, uint64_t freq);
    HuffmanNode(std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r);
    [[nodiscard]] bool is_leaf() const noexcept;
};
```

#### Penjelasan Field

| Field | Tipe | Penjelasan |
| :--- | :--- | :--- |
| `symbol` | `uint8_t` | Byte yang diwakili node ini. Hanya bermakna untuk **leaf node** (node daun). Internal node selalu `symbol = 0`. |
| `frequency` | `uint64_t` | Jumlah kemunculan simbol ini. Untuk internal node, nilainya adalah **jumlah frekuensi kedua anak**. |
| `left` | `unique_ptr<HuffmanNode>` | Pointer ke anak kiri (mewakili bit `0`). `nullptr` jika leaf. |
| `right` | `unique_ptr<HuffmanNode>` | Pointer ke anak kanan (mewakili bit `1`). `nullptr` jika leaf. |

#### Implementasi (huffman.cpp)

```cpp
// Constructor untuk LEAF NODE (node daun)
// Dipanggil saat membuat node awal dari tabel frekuensi
HuffmanNode::HuffmanNode(uint8_t sym, uint64_t freq)
    : symbol(sym), frequency(freq) {}
```

**Sintaks `: symbol(sym), frequency(freq)`** — Ini adalah *member initializer list*. Alih-alih meng-assign di body constructor, kita langsung menginisialisasi member pada saat konstruksi. Ini lebih efisien karena menghindari default-construction lalu assignment.

```cpp
// Constructor untuk INTERNAL NODE (node percabangan)
// Dipanggil saat menggabungkan dua node dalam proses greedy
HuffmanNode::HuffmanNode(std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r)
    : symbol(0), frequency(l->frequency + r->frequency),
      left(std::move(l)), right(std::move(r)) {}
```

**`std::move(l)`** — `unique_ptr` tidak bisa di-copy (karena ownership tunggal). `std::move` mentransfer kepemilikan dari parameter `l` ke member `left`. Setelah move, `l` menjadi `nullptr`.

**`l->frequency + r->frequency`** — Frekuensi internal node = jumlah frekuensi kedua anaknya. Ini adalah properti fundamental dari Huffman Tree.

```cpp
// Mengecek apakah node ini adalah daun (tidak punya anak)
bool HuffmanNode::is_leaf() const noexcept {
    return !left && !right;
}
```

**`const noexcept`** — `const` berarti fungsi ini tidak mengubah state objek. `noexcept` berarti fungsi ini dijamin tidak melempar exception, sehingga compiler bisa mengoptimasi lebih agresif.

**`[[nodiscard]]`** (di deklarasi) — Atribut C++17 yang memaksa pemanggil menggunakan return value. Jika kita menulis `node.is_leaf();` tanpa menyimpan hasilnya, compiler akan memberi warning.

---

### 4.3 HuffmanTree

#### `build()` — Konstruksi Pohon (Greedy Algorithm + Priority Queue)

Ini adalah **fungsi terpenting** di seluruh proyek. Fungsi ini mengimplementasikan algoritma Huffman yang merupakan **greedy algorithm** menggunakan **min-heap (priority queue)**.

```cpp
void HuffmanTree::build(const ByteFrequencyMap& frequencies) {
    // Base case: jika tidak ada simbol, pohon kosong
    if (frequencies.empty()) {
        root_ = nullptr;
        return;
    }
```

```cpp
    // COMPARATOR untuk priority queue (min-heap)
    // Priority queue C++ secara default adalah MAX-heap,
    // jadi kita MEMBALIK perbandingan (a > b) agar jadi MIN-heap
    auto cmp = [](const std::unique_ptr<HuffmanNode>& a,
                  const std::unique_ptr<HuffmanNode>& b) {
        // Prioritas utama: frekuensi lebih KECIL = prioritas lebih TINGGI
        if (a->frequency != b->frequency) return a->frequency > b->frequency;
        // Tie-breaker: jika frekuensi sama, simbol lebih KECIL duluan
        return a->symbol > b->symbol;
    };
```

**Lambda `[](...)  { ... }`** — Fungsi anonim yang didefinisikan inline. `[]` adalah *capture list* (kosong = tidak menangkap variabel luar). Lambda ini digunakan sebagai custom comparator.

**Mengapa `a > b` bukan `a < b`?** — `std::priority_queue` di C++ secara default menggunakan `std::less` yang menghasilkan **max-heap** (elemen terbesar di atas). Untuk mendapatkan **min-heap** (elemen terkecil di atas, yang kita butuhkan untuk Huffman), kita membalik perbandingannya.

```cpp
    // Deklarasi priority queue dengan custom comparator
    std::priority_queue<
        std::unique_ptr<HuffmanNode>,              // Tipe elemen
        std::vector<std::unique_ptr<HuffmanNode>>,  // Container internal
        decltype(cmp)                               // Tipe comparator
    > pq(cmp);
```

**`decltype(cmp)`** — Keyword C++11 yang menghasilkan tipe dari ekspresi `cmp`. Karena lambda memiliki tipe unik yang tidak bisa ditulis manual, kita gunakan `decltype` untuk mendapatkannya.

```cpp
    // FIX DETERMINISME: Salin ke vector dan sort berdasarkan symbol
    // SEBELUM push ke priority queue.
    //
    // MENGAPA? unordered_map tidak menjamin urutan iterasi.
    // Saat compress, map diisi dari scan file (urutan kemunculan pertama).
    // Saat extract, map diisi dari header arsip (urutan berbeda).
    // Urutan push yang berbeda menghasilkan tie-breaking berbeda
    // untuk internal node dengan frekuensi sama, menghasilkan
    // pohon Huffman yang BERBEDA meskipun data frekuensinya IDENTIK.
    std::vector<std::pair<uint8_t, uint64_t>> sorted_freq(
        frequencies.begin(), frequencies.end()
    );
    std::sort(sorted_freq.begin(), sorted_freq.end(),
              [](const auto& a, const auto& b) { return a.first < b.first; });
```

**`frequencies.begin(), frequencies.end()`** — Range constructor: menyalin semua elemen dari `unordered_map` ke `vector`.

**`a.first < b.first`** — Mengurutkan berdasarkan `symbol` (key) secara ascending. Ini memastikan urutan push **selalu sama** terlepas dari bagaimana `unordered_map` menyimpan datanya secara internal.

```cpp
    // Push semua leaf node ke priority queue (dalam urutan deterministik)
    for (const auto& [sym, freq] : sorted_freq) {
        pq.push(std::make_unique<HuffmanNode>(sym, freq));
    }
```

**`const auto& [sym, freq]`** — *Structured binding* (C++17). Secara otomatis meng-decompose `std::pair` menjadi dua variabel `sym` dan `freq`.

**`std::make_unique<HuffmanNode>(sym, freq)`** — Membuat `unique_ptr<HuffmanNode>` baru di heap. Ini lebih aman dan efisien daripada `new HuffmanNode(sym, freq)`.

```cpp
    // === INTI ALGORITMA GREEDY ===
    // Ulangi sampai tersisa satu node (root):
    //   1. Ambil 2 node dengan frekuensi terkecil
    //   2. Gabungkan menjadi satu internal node baru
    //   3. Masukkan kembali ke priority queue
    while (pq.size() > 1) {
        auto left  = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
        pq.pop();
        auto right = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
        pq.pop();
        pq.push(std::make_unique<HuffmanNode>(std::move(left), std::move(right)));
    }
```

**Mengapa `const_cast`?** — `pq.top()` mengembalikan `const&`. Namun, kita perlu `std::move` dari elemen tersebut (karena `unique_ptr` tidak bisa di-copy). `const_cast` menghapus `const` agar `std::move` bisa bekerja. Ini aman karena kita langsung `pop()` setelahnya.

**Greedy Choice:** Pada setiap iterasi, kita **selalu** mengambil dua node dengan frekuensi terkecil. Ini adalah *greedy choice* yang terbukti secara matematis menghasilkan kode **optimal** (tidak ada prefix code lain yang menghasilkan rata-rata panjang kode lebih pendek).

```cpp
    // Node terakhir yang tersisa adalah ROOT dari pohon Huffman
    root_ = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
    pq.pop();
}
```

#### `generate_codes()` — Menghasilkan Kode Prefix-Free

```cpp
HuffmanCodeMap HuffmanTree::generate_codes() const {
    HuffmanCodeMap codes;
    if (!root_) return codes;

    std::vector<bool> current_code;

    // Edge case: jika pohon hanya punya satu leaf (hanya 1 simbol unik)
    // Berikan kode "0" secara manual
    if (root_->is_leaf()) {
        codes[root_->symbol] = {false};  // false = bit 0
        return codes;
    }

    generate_codes_impl(root_.get(), current_code, codes);
    return codes;
}
```

**`root_.get()`** — `unique_ptr::get()` mengembalikan raw pointer tanpa melepas kepemilikan. Digunakan karena fungsi rekursif hanya perlu *observe*, bukan *own* node.

```cpp
// Fungsi rekursif DFS (Depth-First Search) untuk traversal pohon
void HuffmanTree::generate_codes_impl(
    const HuffmanNode* node,
    std::vector<bool>& current_code,
    HuffmanCodeMap& codes
) {
    if (!node) return;

    // Jika mencapai leaf node, simpan kode saat ini
    if (node->is_leaf()) {
        codes[node->symbol] = current_code;
        return;
    }

    // Traversal ke KIRI = tambahkan bit 0
    current_code.push_back(false);
    generate_codes_impl(node->left.get(), current_code, codes);
    current_code.pop_back();  // Backtrack (hapus bit terakhir)

    // Traversal ke KANAN = tambahkan bit 1
    current_code.push_back(true);
    generate_codes_impl(node->right.get(), current_code, codes);
    current_code.pop_back();  // Backtrack
}
```

**Teknik Backtracking:** `push_back` menambah bit sebelum masuk ke subtree, `pop_back` menghapusnya setelah kembali. Dengan cara ini, `current_code` selalu berisi jalur dari root ke node saat ini.

**Properti Prefix-Free:** Karena kode hanya di-assign ke **leaf node**, dan tidak ada leaf yang merupakan ancestor dari leaf lain, maka tidak ada kode yang merupakan prefix dari kode lain. Ini menjamin **decoding yang unambiguous**.

#### Fungsi Utilitas Pohon

```cpp
// Menghitung tinggi pohon secara rekursif
int HuffmanTree::height_impl(const HuffmanNode* node) noexcept {
    if (!node) return 0;
    if (node->is_leaf()) return 1;
    return 1 + std::max(
        height_impl(node->left.get()),
        height_impl(node->right.get())
    );
}

// Menghitung jumlah total node secara rekursif
int HuffmanTree::node_count_impl(const HuffmanNode* node) noexcept {
    if (!node) return 0;
    return 1 + node_count_impl(node->left.get())
             + node_count_impl(node->right.get());
}
```

Kedua fungsi menggunakan **rekursi** yang merupakan teknik natural untuk traversal tree. Kompleksitas keduanya $O(n)$ dimana $n$ = jumlah node.

---

### 4.4 BitWriter & BitReader

Karena kode Huffman memiliki panjang variabel (misalnya 3 bit, 5 bit, 7 bit), kita tidak bisa langsung menulis ke file yang bekerja dalam satuan **byte** (8 bit). Kelas `BitWriter` dan `BitReader` menjembatani gap ini.

#### BitWriter — Menulis Bit per Bit ke Output Stream

```cpp
class BitWriter {
private:
    std::ostream& out_;          // Reference ke output stream
    uint8_t  buffer_     = 0;    // Buffer 8-bit yang sedang diisi
    int      bit_count_  = 0;    // Berapa bit sudah terisi di buffer
    uint64_t bytes_written_ = 0; // Counter total byte yang ditulis
};
```

**`std::ostream& out_`** — Reference (bukan pointer, bukan copy). Ini berarti `BitWriter` tidak memiliki stream-nya, hanya merujuk ke stream yang sudah ada. Reference harus diinisialisasi saat konstruksi dan tidak bisa di-reassign.

```cpp
void BitWriter::write_bit(bool bit) {
    // Geser buffer ke kiri 1 posisi, lalu masukkan bit baru di posisi LSB
    buffer_ = static_cast<uint8_t>((buffer_ << 1) | (bit ? 1 : 0));
    ++bit_count_;

    // Jika buffer sudah penuh (8 bit), tulis ke stream
    if (bit_count_ == 8) {
        out_.put(static_cast<char>(buffer_));
        ++bytes_written_;
        buffer_    = 0;
        bit_count_ = 0;
    }
}
```

**Visualisasi proses buffer:**
```text
Menulis bit: 1, 0, 1, 1, 0, 0, 1, 0

Step 1: buffer = 00000001  (bit_count = 1)
Step 2: buffer = 00000010  (bit_count = 2)
Step 3: buffer = 00000101  (bit_count = 3)
Step 4: buffer = 00001011  (bit_count = 4)
Step 5: buffer = 00010110  (bit_count = 5)
Step 6: buffer = 00101100  (bit_count = 6)
Step 7: buffer = 01011001  (bit_count = 7)
Step 8: buffer = 10110010  (bit_count = 8) → TULIS KE STREAM → reset
```

**`static_cast<uint8_t>(...)`** — Konversi tipe eksplisit. Tanpa cast, operasi bitwise pada tipe kecil (uint8_t) akan di-promote ke `int` oleh compiler. Cast memastikan hasilnya kembali ke `uint8_t`.

```cpp
uint8_t BitWriter::flush() {
    if (bit_count_ == 0) return 0;  // Tidak ada sisa bit

    // Hitung berapa bit padding yang ditambahkan
    uint8_t padding = static_cast<uint8_t>(8 - bit_count_);

    // Geser sisa bit ke posisi MSB dan isi sisanya dengan 0
    buffer_ = static_cast<uint8_t>(buffer_ << padding);

    out_.put(static_cast<char>(buffer_));
    ++bytes_written_;
    buffer_    = 0;
    bit_count_ = 0;

    return padding;  // Informasi ini disimpan di header .puff
}
```

**Mengapa perlu padding?** Karena file hanya bisa menyimpan byte utuh. Jika data terkompresi berakhir di tengah-tengah byte (misalnya hanya 5 bit terisi), kita harus menambah 3 bit padding (nol) agar menjadi 8 bit penuh. Jumlah padding disimpan di header arsip agar decoder tahu kapan harus berhenti.

#### BitReader — Membaca Bit per Bit dari Input Stream

```cpp
std::optional<bool> BitReader::read_bit() {
    // Jika buffer kosong, baca byte baru dari stream
    if (bits_remaining_ == 0) {
        char ch;
        if (!in_.get(ch)) return std::nullopt;  // EOF
        buffer_         = static_cast<uint8_t>(ch);
        bits_remaining_ = 8;
    }
    --bits_remaining_;
    // Ekstrak bit dari posisi MSB
    return (buffer_ >> bits_remaining_) & 1;
}
```

**`std::optional<bool>`** — Tipe C++17 yang bisa berisi `bool` atau *nothing* (`std::nullopt`). Digunakan untuk menandakan EOF dengan elegan tanpa perlu flag atau exception.

**`(buffer_ >> bits_remaining_) & 1`** — Mengekstrak satu bit dari posisi `bits_remaining_`. Operasi `>> n` menggeser bit ke kanan sebanyak `n`, lalu `& 1` mengambil hanya bit paling kanan (LSB).

---

### 4.5 Encoder & Decoder

#### Encoder::calculate_frequencies()

```cpp
ByteFrequencyMap Encoder::calculate_frequencies(std::istream& input) {
    ByteFrequencyMap freq;
    char ch;
    // Baca byte demi byte dan hitung kemunculannya
    while (input.get(ch)) {
        ++freq[static_cast<uint8_t>(ch)];
    }
    return freq;
}
```

**`input.get(ch)`** — Membaca satu karakter dari stream. Mengembalikan referensi ke stream itu sendiri, yang bernilai `true` jika berhasil, `false` jika EOF.

**`++freq[static_cast<uint8_t>(ch)]`** — `unordered_map::operator[]` akan membuat entry baru dengan value `0` jika key belum ada. Kemudian `++` menambah 1. Ini adalah idiom standar C++ untuk menghitung frekuensi.

**Kompleksitas:** $O(n)$ dimana $n$ = jumlah byte dalam file. Setiap byte di-lookup dan di-increment dalam $O(1)$ rata-rata.

#### Encoder::encode()

```cpp
EncodeResult Encoder::encode(
    std::istream& input,
    std::ostream& output,
    const HuffmanCodeMap& codes
) {
    BitWriter writer(output);
    char ch;
    while (input.get(ch)) {
        // Cari kode Huffman untuk byte ini
        auto it = codes.find(static_cast<uint8_t>(ch));
        if (it != codes.end()) {
            // Tulis semua bit dari kode Huffman
            writer.write_bits(it->second);
        }
    }
    uint8_t padding = writer.flush();
    return {writer.bytes_written(), padding};
}
```

**`codes.find(...)` vs `codes[...]`** — `find` tidak memodifikasi map (mengembalikan iterator), sementara `operator[]` bisa insert entry baru. Karena `codes` adalah `const`, kita harus menggunakan `find`.

**`it->second`** — Iterator ke `unordered_map` menunjuk ke `std::pair<const Key, Value>`. `it->second` adalah value-nya, yaitu `vector<bool>` berisi kode Huffman.

**`return {writer.bytes_written(), padding}`** — *Aggregate initialization* C++11. Membuat `EncodeResult` secara langsung dari dua nilai tanpa perlu menyebut nama struct.

#### Decoder::decode()

```cpp
void Decoder::decode(
    std::istream& input,
    std::ostream& output,
    const HuffmanTree& tree,
    uint64_t original_size
) {
    if (original_size == 0 || tree.empty()) return;

    const HuffmanNode* root = tree.get_root();
    BitReader reader(input);

    // Edge case: hanya satu simbol unik
    if (root->is_leaf()) {
        for (uint64_t i = 0; i < original_size; ++i) {
            output.put(static_cast<char>(root->symbol));
            reader.read_bit();  // Konsumsi bit (meski tidak dibutuhkan)
        }
        return;
    }

    uint64_t decoded = 0;
    const HuffmanNode* current = root;

    while (decoded < original_size) {
        auto bit = reader.read_bit();
        if (!bit.has_value()) {
            throw std::runtime_error("Unexpected end of bitstream");
        }

        // bit = 0 → ke kiri, bit = 1 → ke kanan
        current = bit.value() ? current->right.get() : current->left.get();

        if (!current) {
            throw std::runtime_error("Invalid Huffman tree path");
        }

        // Jika mencapai leaf, outputkan simbol dan kembali ke root
        if (current->is_leaf()) {
            output.put(static_cast<char>(current->symbol));
            ++decoded;
            current = root;  // Reset ke root untuk decode simbol berikutnya
        }
    }
}
```

**Proses Decoding (Traversal Pohon):**

Decoder membaca bitstream bit-per-bit dan berjalan menuruni pohon Huffman:
- Bit `0` → belok kiri
- Bit `1` → belok kanan
- Saat mencapai **leaf node** → outputkan simbol, kembali ke root

Ini bekerja karena kode Huffman bersifat **prefix-free**: tidak ada kode yang merupakan awalan dari kode lain, sehingga setiap kali kita mencapai leaf, kita tahu pasti kita telah selesai membaca satu simbol utuh.

**`bit.value()`** — Mengambil nilai `bool` dari `std::optional<bool>`. Hanya aman dipanggil setelah `has_value()` bernilai `true`.

**`original_size`** — Kritis untuk mengetahui kapan berhenti. Tanpa ini, decoder akan terus membaca padding bits di akhir dan menghasilkan byte sampah.

---

## 5. Module 2: Archive

File: `include/archive.hpp` (deklarasi) + `src/archive.cpp` (implementasi)

Modul ini menangani **serialisasi** (menulis data terstruktur ke file biner) dan **deserialisasi** (membaca kembali).

---

### 5.1 Format Biner .puff

```text
┌──────────────────────────────────────────────────────────────┐
│                     HEADER                                   │
├──────────┬───────────┬───────────────────────────────────────┤
│ Offset   │ Ukuran    │ Isi                                   │
├──────────┼───────────┼───────────────────────────────────────┤
│ 0x00     │ 4 byte    │ Magic Number: "PUFF" (identifikasi)   │
│ 0x04     │ 1 byte    │ Version Major (0x01)                  │
│ 0x05     │ 1 byte    │ Version Minor (0x00)                  │
│ 0x06     │ 2 byte    │ Panjang nama file asli (little-endian)│
│ 0x08     │ N byte    │ Nama file asli (string UTF-8)         │
│ 0x08+N   │ 8 byte    │ Ukuran file asli (little-endian)      │
│          │ 2 byte    │ Jumlah simbol unik                    │
│          │ 9×S byte  │ Tabel frekuensi: S × (1 + 8) byte    │
│          │ 1 byte    │ Jumlah padding bits (0–7)             │
├──────────┼───────────┼───────────────────────────────────────┤
│                     DATA                                     │
├──────────┼───────────┼───────────────────────────────────────┤
│          │ sisanya   │ Bitstream data terkompresi             │
└──────────┴───────────┴───────────────────────────────────────┘
```

### Konstanta Format

```cpp
inline constexpr char    PUFF_MAGIC[4]      = {'P', 'U', 'F', 'F'};
inline constexpr uint8_t PUFF_VERSION_MAJOR = 0x01;
inline constexpr uint8_t PUFF_VERSION_MINOR = 0x00;
```

**`inline constexpr`** — `constexpr` berarti nilai diketahui saat kompilasi. `inline` memungkinkan definisi di file header tanpa melanggar ODR (One Definition Rule) saat di-include dari banyak `.cpp`.

---

### 5.2 Helper Fungsi Serialisasi

```cpp
// Menulis integer dalam format LITTLE-ENDIAN (byte paling rendah ditulis pertama)
static void write_u16(std::ostream& out, uint16_t v) {
    out.put(static_cast<char>(v & 0xFF));         // Byte rendah
    out.put(static_cast<char>((v >> 8) & 0xFF));  // Byte tinggi
}

static void write_u64(std::ostream& out, uint64_t v) {
    for (int i = 0; i < 8; ++i)
        out.put(static_cast<char>((v >> (8 * i)) & 0xFF));
}
```

**Little-Endian:** Byte paling tidak signifikan (least significant byte) ditulis terlebih dahulu. Contoh: angka `0x1234` ditulis sebagai byte `0x34` diikuti `0x12`. Format ini dipilih karena umum digunakan di arsitektur x86.

**`v & 0xFF`** — Operasi bitwise AND yang mengambil 8 bit paling rendah dari `v`.

**`(v >> 8) & 0xFF`** — Geser `v` ke kanan 8 bit, lalu ambil 8 bit terendah. Hasilnya adalah byte kedua dari `v`.

**`static`** — Membatasi visibilitas fungsi hanya di dalam file ini (*internal linkage*). Fungsi ini tidak bisa dipanggil dari file `.cpp` lain.

```cpp
// Membaca integer dari format little-endian
static uint8_t read_u8(std::istream& in) {
    char ch;
    if (!in.get(ch)) throw std::runtime_error("Unexpected EOF");
    return static_cast<uint8_t>(ch);
}

static uint64_t read_u64(std::istream& in) {
    uint64_t v = 0;
    for (int i = 0; i < 8; ++i)
        v |= static_cast<uint64_t>(read_u8(in)) << (8 * i);
    return v;
}
```

**`v |= ... << (8 * i)`** — Membaca byte ke-`i`, menggesernya ke posisi yang tepat, lalu meng-OR-kan dengan akumulator `v`. Setelah 8 iterasi, `v` berisi integer 64-bit lengkap.

---

### 5.3 ArchiveWriter::compress()

```cpp
void ArchiveWriter::compress(const fs::path& file_path) {
    // 1. VALIDASI: Pastikan file ada dan merupakan file reguler
    if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
        throw std::runtime_error("Not a regular file: " + file_path.string());
    }

    // 2. BACA seluruh file ke memory
    std::ifstream in(file_path, std::ios::binary);
    std::ostringstream raw_buf;
    raw_buf << in.rdbuf();                    // Salin seluruh isi stream
    std::string raw_data = raw_buf.str();

    // 3. HITUNG frekuensi setiap byte
    std::istringstream freq_stream(raw_data);
    ByteFrequencyMap frequencies = Encoder::calculate_frequencies(freq_stream);

    // 4. BANGUN pohon Huffman dari tabel frekuensi
    HuffmanTree tree;
    tree.build(frequencies);
    HuffmanCodeMap codes = tree.generate_codes();

    // 5. ENCODE: ganti setiap byte dengan kode Huffman-nya
    std::istringstream encode_stream(raw_data);
    std::ostringstream compressed_buf;
    EncodeResult result = Encoder::encode(encode_stream, compressed_buf, codes);
    std::string compressed_data = compressed_buf.str();

    // 6. TULIS file .puff
    fs::path output_path = file_path;
    output_path.replace_extension(".puff");

    std::ofstream out(output_path, std::ios::binary);

    // Header
    write_bytes(out, PUFF_MAGIC, 4);          // "PUFF"
    write_u8(out, PUFF_VERSION_MAJOR);        // 0x01
    write_u8(out, PUFF_VERSION_MINOR);        // 0x00

    std::string filename = file_path.filename().string();
    write_u16(out, static_cast<uint16_t>(filename.size()));
    write_bytes(out, filename.data(), filename.size());

    write_u64(out, raw_data.size());          // Ukuran asli

    // Tabel frekuensi
    write_u16(out, static_cast<uint16_t>(frequencies.size()));
    for (const auto& [sym, freq] : frequencies) {
        write_u8(out, sym);
        write_u64(out, freq);
    }

    write_u8(out, result.padding_bits);       // Jumlah padding

    // Data terkompresi
    write_bytes(out, compressed_data.data(), compressed_data.size());
    out.flush();

    // 7. TAMPILKAN hasil ke user
    double ratio = (1.0 - static_cast<double>(compressed_data.size()) /
                          static_cast<double>(raw_data.size())) * 100.0;

    std::cout << "Compressed: " << file_path.filename().string()
              << " -> " << output_path.filename().string() << "\n"
              << "  Original:   " << raw_data.size() << " bytes\n"
              << "  Compressed: " << compressed_data.size() << " bytes\n"
              << "  Ratio:      " << std::fixed << std::setprecision(1)
              << ratio << "% reduction\n";
}
```

**`std::ios::binary`** — Membuka file dalam mode biner. Tanpa flag ini, pada Windows, karakter `\n` (0x0A) akan dikonversi menjadi `\r\n` (0x0D 0x0A), yang akan merusak data biner.

**`raw_buf << in.rdbuf()`** — Menyalin seluruh isi `ifstream` ke `ostringstream`. `rdbuf()` mengembalikan buffer internal dari stream.

**`file_path.replace_extension(".puff")`** — Method dari `std::filesystem::path` yang mengganti ekstensi file. `sample.txt` → `sample.puff`.

---

### 5.4 ArchiveReader::extract()

```cpp
void ArchiveReader::extract(const fs::path& archive_path) {
    std::ifstream in(archive_path, std::ios::binary);

    // 1. VALIDASI magic number
    char magic[4]{};
    if (!in.read(magic, 4) || std::memcmp(magic, PUFF_MAGIC, 4) != 0) {
        throw std::runtime_error("Invalid archive format");
    }

    // 2. BACA versi
    uint8_t major = read_u8(in);
    uint8_t minor = read_u8(in);
    (void)minor;  // Suppress "unused variable" warning

    if (major != PUFF_VERSION_MAJOR) {
        throw std::runtime_error("Unsupported archive version");
    }

    // 3. BACA nama file asli
    uint16_t name_len = read_u16(in);
    std::string original_filename(name_len, '\0');
    in.read(original_filename.data(), name_len);

    // 4. BACA ukuran asli
    uint64_t original_size = read_u64(in);

    // 5. BACA tabel frekuensi & REBUILD pohon Huffman
    uint16_t unique_symbols = read_u16(in);
    ByteFrequencyMap frequencies;
    for (uint16_t i = 0; i < unique_symbols; ++i) {
        uint8_t  sym  = read_u8(in);
        uint64_t freq = read_u64(in);
        frequencies[sym] = freq;
    }

    read_u8(in);  // padding_bits (tidak digunakan langsung — decoder berhenti
                  // berdasarkan original_size)

    HuffmanTree tree;
    tree.build(frequencies);  // Rebuild pohon yang IDENTIK dengan saat compress

    // 6. BACA sisa data (bitstream terkompresi)
    std::ostringstream rest;
    rest << in.rdbuf();
    std::string compressed_data = rest.str();

    // 7. DECODE
    fs::path output_path = archive_path.parent_path() / original_filename;
    std::istringstream compressed_stream(compressed_data);
    std::ofstream out(output_path, std::ios::binary);

    Decoder::decode(compressed_stream, out, tree, original_size);

    std::cout << "Extracted: " << archive_path.filename().string()
              << " -> " << original_filename << "\n"
              << "  Size: " << original_size << " bytes\n";
}
```

**`std::memcmp(magic, PUFF_MAGIC, 4)`** — Membandingkan 4 byte pertama file dengan "PUFF". Jika hasilnya `!= 0`, file bukan arsip `.puff` yang valid.

**`(void)minor`** — Idiom C++ untuk mensupresi compiler warning tentang variabel yang tidak digunakan. Kita membaca `minor` dari stream (karena harus mengonsumsi byte tersebut), tapi saat ini tidak menggunakannya untuk logika apapun.

**`archive_path.parent_path() / original_filename`** — Operator `/` pada `std::filesystem::path` menggabungkan path. Jika arsip berada di `samples/data.puff` dan nama aslinya `data.txt`, hasilnya `samples/data.txt`.

**Kunci Lossless:** Fungsi `tree.build(frequencies)` di sini akan menghasilkan pohon yang **100% identik** dengan pohon yang dibangun saat compress, karena:
1. Tabel frekuensi disimpan secara eksak di header arsip
2. Fungsi `build()` mengurutkan frekuensi secara deterministik sebelum memasukkannya ke priority queue

---

## 6. Module 3: Statistics

File: `include/statistics.hpp` (deklarasi) + `src/statistics.cpp` (implementasi)

Modul ini menghitung **metrik matematis** dari Information Theory.

---

### 6.1 AnalysisResult

```cpp
struct AnalysisResult {
    std::string filename;
    uint64_t total_symbols  = 0;   // Jumlah total byte dalam file
    uint16_t unique_symbols = 0;   // Jumlah simbol unik (karakter berbeda)
    int tree_height         = 0;   // Tinggi pohon Huffman
    int total_nodes         = 0;   // Jumlah node dalam pohon
    double entropy          = 0.0; // Shannon Entropy (bits/symbol)
    double avg_code_length  = 0.0; // Rata-rata panjang kode Huffman
    double efficiency       = 0.0; // (entropy / avg_code_length) × 100%

    struct SymbolInfo {
        uint8_t     symbol;        // Byte simbol
        uint64_t    frequency;     // Berapa kali muncul
        double      percentage;    // Persentase kemunculan
        std::string code;          // Kode Huffman dalam bentuk string "010110"
    };
    std::vector<SymbolInfo> top_symbols;  // 10 simbol paling sering
};
```

Struct ini adalah **data container** murni — tidak memiliki logika, hanya menyimpan hasil kalkulasi agar bisa dioper ke fungsi `print_report()`.

---

### 6.2 Statistics::analyze()

```cpp
AnalysisResult Statistics::analyze(const fs::path& file_path) {
    // ... (baca file, hitung frekuensi, bangun tree — sama seperti compress)

    // === SHANNON ENTROPY ===
    // H = -Σ p(x) × log₂(p(x))
    // dimana p(x) = frekuensi(x) / total_symbols
    double entropy = 0.0;
    for (const auto& [sym, freq] : frequencies) {
        double p = static_cast<double>(freq) / static_cast<double>(result.total_symbols);
        if (p > 0) entropy -= p * std::log2(p);
    }
    result.entropy = entropy;
```

**Shannon Entropy** adalah konsep fundamental dari Information Theory (Claude Shannon, 1948). Entropy mengukur **rata-rata jumlah informasi minimum** (dalam bit) yang diperlukan untuk merepresentasikan setiap simbol.

**Formula:** $H = -\sum_{x \in X} p(x) \cdot \log_2 p(x)$

- $p(x)$ = probabilitas kemunculan simbol $x$ = `freq / total`
- $\log_2 p(x)$ = jumlah bit yang dibutuhkan jika hanya ada simbol $x$
- Tanda negatif karena $\log_2$ dari angka antara 0 dan 1 selalu negatif

**Contoh:** Jika `'e'` muncul 10% dari waktu, maka $p(e) = 0.1$ dan kontribusinya ke entropy = $-0.1 \times \log_2(0.1) = -0.1 \times (-3.32) = 0.332$ bits.

**`if (p > 0)`** — Karena $\log_2(0)$ adalah *undefined* (negatif tak hingga), kita melewati simbol dengan frekuensi 0.

```cpp
    // === AVERAGE CODE LENGTH ===
    // L = Σ p(x) × |code(x)|
    // dimana |code(x)| adalah panjang kode Huffman untuk simbol x
    double avg_len = 0.0;
    for (const auto& [sym, freq] : frequencies) {
        double p = static_cast<double>(freq) / static_cast<double>(result.total_symbols);
        avg_len += p * static_cast<double>(codes[sym].size());
    }
    result.avg_code_length = avg_len;
```

**Average Code Length** ($L$) adalah **rata-rata tertimbang** dari panjang semua kode Huffman. Menurut **Source Coding Theorem** Shannon, untuk kode prefix-free optimal:

$$H \leq L < H + 1$$

Artinya, kode Huffman mendekati batas teoritisnya — tidak bisa lebih baik dari entropy.

```cpp
    // === COMPRESSION EFFICIENCY ===
    // efficiency = (H / L) × 100%
    result.efficiency = (result.avg_code_length > 0)
        ? (result.entropy / result.avg_code_length) * 100.0
        : 0.0;
```

**Compression Efficiency** mengukur seberapa dekat kode Huffman kita dengan batas optimal Shannon Entropy. Efisiensi 100% berarti $L = H$ (sempurna). Dalam praktik, efisiensi Huffman biasanya 95–99.9%.

```cpp
    // Urutkan simbol berdasarkan frekuensi (tertinggi dulu)
    std::vector<std::pair<uint8_t, uint64_t>> freq_vec(
        frequencies.begin(), frequencies.end()
    );
    std::sort(freq_vec.begin(), freq_vec.end(),
              [](const auto& a, const auto& b) { return a.second > b.second; });

    // Ambil 10 simbol teratas
    size_t top_n = std::min<size_t>(10, freq_vec.size());
```

**`std::min<size_t>(10, freq_vec.size())`** — Template explicit instantiation. Tanpa `<size_t>`, compiler bisa bingung karena `10` bertipe `int` dan `freq_vec.size()` bertipe `size_t` (unsigned).

---

### 6.3 Statistics::print_report()

```cpp
void Statistics::print_report(const AnalysisResult& result) {
    // Lambda untuk memformat simbol agar mudah dibaca
    auto format_symbol = [](uint8_t sym) -> std::string {
        if (sym == ' ') return "' '";       // Spasi
        if (sym == '\n') return "'\\n'";     // Newline
        if (sym >= 33 && sym < 127)          // Printable ASCII
            return std::string("'") + static_cast<char>(sym) + "'";
        char buf[8];
        std::snprintf(buf, sizeof(buf), "0x%02X", sym);  // Non-printable → hex
        return buf;
    };
```

**`-> std::string`** — *Trailing return type* pada lambda. Memberi tahu compiler secara eksplisit tipe kembalian lambda.

**`std::snprintf(buf, sizeof(buf), "0x%02X", sym)`** — Fungsi format ala C yang menulis ke buffer. `%02X` berarti: hexadecimal, minimal 2 digit, padded dengan nol. Contoh: byte `0x0A` ditulis sebagai `"0x0A"`.

```cpp
    // Lambda untuk memformat angka dengan pemisah ribuan
    auto format_num = [](uint64_t n) {
        std::string s = std::to_string(n);
        int p = static_cast<int>(s.length()) - 3;
        while (p > 0) { s.insert(p, ","); p -= 3; }
        return s;
    };
```

**Contoh:** `1496` → `"1,496"`, `1000000` → `"1,000,000"`.

```cpp
    // Output tabel dengan border Unicode
    std::cout << "\n"
        << "═══════════════════════════════════════════════════\n"
        << "  Pufferfish — Huffman Analysis\n"
        << "═══════════════════════════════════════════════════\n"
        << "  File:                    " << result.filename << "\n"
        // ... (field lainnya)
        << std::fixed << std::setprecision(2)
        << "  Shannon Entropy:         " << result.entropy << " bits/symbol\n"
        << "  Average Code Length:     " << result.avg_code_length << " bits/symbol\n"
        << "  Compression Efficiency:  " << std::setprecision(1)
        << result.efficiency << "%\n";
```

**`std::fixed`** — Menampilkan angka desimal dalam notasi tetap (bukan scientific). Contoh: `4.65` bukan `4.65e+00`.

**`std::setprecision(2)`** — Menampilkan 2 digit di belakang koma. Efeknya persisten (berlaku sampai diubah lagi).

**`std::setw(6)`** — Mengatur lebar minimum kolom menjadi 6 karakter. Efeknya hanya untuk satu output berikutnya.

**`std::left` / `std::right`** — Mengatur alignment teks dalam kolom. `left` untuk rata kiri, `right` untuk rata kanan.

---

## 7. Module 4: CLI Entry Point

File: `src/main.cpp`

```cpp
#include "archive.hpp"
#include "statistics.hpp"

namespace pufferfish {
    void print_usage(const std::string& program) {
        std::cout << R"(Pufferfish — Educational Huffman Coding Utility
        // ... help text
        )";
    }
} // namespace pufferfish
```

**`R"(...)"` — Raw string literal** (C++11). Memungkinkan menulis string multi-baris tanpa escape character. Semua yang ada di antara `R"(` dan `)"` dianggap karakter literal, termasuk newline dan tanda kutip.

```cpp
int main(int argc, char* argv[]) {
    // argc = jumlah argumen (termasuk nama program)
    // argv = array of C-strings berisi argumen
    // Contoh: "./puff compress file.txt"
    //   argc = 3
    //   argv[0] = "./puff"
    //   argv[1] = "compress"
    //   argv[2] = "file.txt"

    if (argc < 2) {
        pufferfish::print_usage(argv[0]);
        return 1;  // Exit code non-zero = error
    }

    std::string cmd = argv[1];

    // Shorthand aliases
    if (cmd == "c") cmd = "compress";
    if (cmd == "x") cmd = "extract";
    if (cmd == "a") cmd = "analyze";

    try {
        if (cmd == "compress") {
            if (argc < 3) throw std::runtime_error("Requires file argument");
            pufferfish::ArchiveWriter::compress(argv[2]);
            return 0;
        }
        // ... (extract, analyze, help — pola yang sama)

        throw std::runtime_error("Unknown command: " + cmd);

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
```

**`try { ... } catch (const std::exception& e)`** — Blok *exception handling*. Semua error dari modul-modul di bawah (runtime_error) akan ditangkap di sini dan ditampilkan ke user secara rapi via `stderr`.

**`std::cerr`** — Output stream untuk error messages. Tidak di-buffer (langsung flush), dan bisa dipisahkan dari `stdout` di shell (`2>/dev/null`).

**`return 0` vs `return 1`** — Konvensi Unix: `0` = sukses, non-zero = error. Shell dan script bisa mengecek exit code ini.

---

## 8. Konsep Matematika dalam Kode

### Ringkasan Pemetaan Teori → Kode

| Konsep Matematika | Lokasi dalam Kode | Fungsi/Kelas |
| :--- | :--- | :--- |
| **Binary Tree** | `huffman.hpp` | `HuffmanNode` (struct dengan left/right pointer) |
| **Min-Heap (Priority Queue)** | `huffman.cpp` | `std::priority_queue` dengan custom comparator di `build()` |
| **Greedy Algorithm** | `huffman.cpp` | Loop `while (pq.size() > 1)` yang selalu mengambil 2 node terkecil |
| **Prefix-Free Code** | `huffman.cpp` | `generate_codes_impl()` — kode hanya di-assign ke leaf node |
| **Shannon Entropy** | `statistics.cpp` | `analyze()` — formula $H = -\sum p \log_2 p$ |
| **Lossless Compression** | `archive.cpp` | Round-trip: `compress()` lalu `extract()` menghasilkan file identik |
| **Information Encoding** | `huffman.cpp` | `Encoder::encode()` — mengganti byte tetap 8-bit dengan kode variabel |
| **DFS (Depth-First Search)** | `huffman.cpp` | `generate_codes_impl()` — traversal rekursif |
| **Bitwise Operations** | `huffman.cpp` | `BitWriter` dan `BitReader` — shift, AND, OR |
| **Serialisasi Biner** | `archive.cpp` | `write_u16()`, `write_u64()`, `read_u16()`, `read_u64()` |

---

## 9. Glossary Sintaks C++ yang Digunakan

Referensi cepat untuk sintaks C++ modern yang digunakan di proyek ini.

| Sintaks | Standar | Penjelasan | Contoh di Proyek |
| :--- | :--- | :--- | :--- |
| `auto` | C++11 | Deduksi tipe otomatis | `auto cmp = [](...){}` |
| `[](auto& a){ ... }` | C++11/14 | Lambda expression | Comparator di `build()` |
| `std::unique_ptr<T>` | C++11 | Smart pointer dengan kepemilikan tunggal | `HuffmanNode::left`, `right` |
| `std::make_unique<T>(...)` | C++14 | Membuat unique_ptr secara aman | `pq.push(make_unique<HuffmanNode>(...))` |
| `std::move(x)` | C++11 | Mentransfer kepemilikan resource | Move node antar tree |
| `const auto& [k, v]` | C++17 | Structured binding | `for (const auto& [sym, freq] : ...)` |
| `std::optional<T>` | C++17 | Tipe yang bisa kosong | `BitReader::read_bit()` return type |
| `std::nullopt` | C++17 | Nilai "kosong" untuk optional | Return saat EOF |
| `[[nodiscard]]` | C++17 | Atribut: return value harus digunakan | `is_leaf()`, `generate_codes()` |
| `constexpr` | C++11 | Evaluasi saat kompilasi | `PUFF_MAGIC`, `PUFF_VERSION_*` |
| `noexcept` | C++11 | Janji tidak melempar exception | `is_leaf()`, `height()` |
| `static_cast<T>(x)` | C++11 | Konversi tipe eksplisit yang aman | Konversi `char` ↔ `uint8_t` |
| `const_cast<T>(x)` | C++11 | Menghapus qualifier `const` | Move dari `pq.top()` |
| `decltype(expr)` | C++11 | Mendapatkan tipe dari ekspresi | `decltype(cmp)` untuk tipe lambda |
| `R"(...)"` | C++11 | Raw string literal (tanpa escape) | Help text di `main.cpp` |
| `#ifndef` / `#define` / `#endif` | C89 | Include guard (mencegah double inclusion) | Setiap `.hpp` |
| `namespace` | C++98 | Pengelompokan nama untuk menghindari konflik | `namespace pufferfish { }` |
| `std::filesystem::path` | C++17 | Representasi path file yang cross-platform | Parameter di `compress()`, `extract()` |
| `try { } catch (const std::exception& e)` | C++98 | Exception handling | `main()` |
