# .puff Archive Format

This document describes the binary layout of `.puff` files produced by Pufferfish.

All examples use the string **`"abracadabra"`** for consistency with [explanation.md](explanation.md).

## Overview

A `.puff` file stores a single compressed file. The format is intentionally minimal — just enough to reconstruct the original file.

All multi-byte integers are stored in **little-endian** byte order.

## Layout

| Offset | Field | Type | Description |
|--------|-------|------|-------------|
| 0 | Magic | 4 bytes | `PUFF` (`0x50 0x55 0x46 0x46`) |
| 4 | Version Major | uint8 | `0x01` |
| 5 | Version Minor | uint8 | `0x00` |
| 6 | Filename Length | uint16 | Length of the original filename |
| 8 | Filename | N bytes | Original filename (UTF-8, e.g. `abracadabra.txt`) |
| 8+N | Original Size | uint64 | Original file size in bytes |
| 16+N | Unique Symbols | uint16 | Number of entries in frequency table |
| 18+N | Frequency Table | 9 × count | Repeated: 1 byte symbol + 8 bytes frequency |
| varies | Padding Bits | uint8 | Number of zero-padding bits in last byte (0–7) |
| varies | Encoded Data | remaining | Huffman-encoded bitstream |

## Sections

### Magic Header (4 bytes)

```
50 55 46 46    "PUFF"
```

Used to identify `.puff` files. If these bytes don't match, the file is not a valid archive.

### Version (2 bytes)

```
01 00          v1.0
```

Major version `0x01`, minor version `0x00`. The reader rejects archives with a different major version.

### Original Filename (2 + N bytes)

The length is stored as a 16-bit integer, followed by the filename string. This is the name used when extracting the file.

```
0F 00                                             length = 15
61 62 72 61 63 61 64 61 62 72 61 2E 74 78 74      "abracadabra.txt"
```

### Original File Size (8 bytes)

The size of the uncompressed file. The decoder uses this to know when to stop — without it, the padding bits at the end of the bitstream would be misinterpreted as data.

```
0B 00 00 00 00 00 00 00    original size = 11
```

### Frequency Table (2 + 9×N bytes)

The number of unique symbols, followed by pairs of (symbol, frequency):

```
05 00                              5 unique symbols
61 05 00 00 00 00 00 00 00         'a' (0x61) appears 5 times
62 02 00 00 00 00 00 00 00         'b' (0x62) appears 2 times
72 02 00 00 00 00 00 00 00         'r' (0x72) appears 2 times
63 01 00 00 00 00 00 00 00         'c' (0x63) appears 1 time
64 01 00 00 00 00 00 00 00         'd' (0x64) appears 1 time
```

The reader uses this table to rebuild the Huffman tree. Because tree construction is deterministic (frequencies are sorted by symbol before insertion into the priority queue, and the comparator uses `symbol` as tie-breaker), the rebuilt tree is identical to the one used during compression.

### Padding Bits (1 byte)

The number of zero bits padded at the end of the last byte of encoded data (0–7). This is necessary because Huffman codes are variable-length and the total number of bits may not be a multiple of 8.

```
01                         1 padding bit
```

### Encoded Data (remaining bytes)

The Huffman-encoded bitstream. Bits are packed MSB-first (most significant bit first).

```
79 8D 78                   3 bytes of encoded data
```

## Complete Example: `"abracadabra"`

Compressing `"abracadabra"` stored as `abracadabra.txt` (11 bytes):

```
Frequencies: a=5, b=2, r=2, c=1, d=1
Codes:       a=0, r=10, b=111, c=1100, d=1101

Encoded: a   b    r   a   c      a   d      a   b    r   a
         0   111  10  0   1100   0   1101   0   111  10  0
       = 01111001 10001101 0111100_   (23 bits → 3 bytes, 1 padding bit)
```

The resulting `.puff` file:

```
50 55 46 46                                        PUFF magic
01 00                                              v1.0
0F 00                                              filename length = 15
61 62 72 61 63 61 64 61 62 72 61 2E 74 78 74       "abracadabra.txt"
0B 00 00 00 00 00 00 00                            original size = 11
05 00                                              5 unique symbols
61 05 00 00 00 00 00 00 00                         'a' = 5
62 02 00 00 00 00 00 00 00                         'b' = 2
72 02 00 00 00 00 00 00 00                         'r' = 2
63 01 00 00 00 00 00 00 00                         'c' = 1
64 01 00 00 00 00 00 00 00                         'd' = 1
01                                                 1 padding bit
79 8D 78                                           encoded data
```

**Total `.puff` size:** 4 + 2 + 2 + 15 + 8 + 2 + 45 + 1 + 3 = **82 bytes**

> **Note:** For small files like `"abracadabra"` (11 bytes), the `.puff` file is actually *larger* than the original because the frequency table overhead (45 bytes for 5 symbols) dominates. Huffman compression becomes effective on larger files where the bitstream savings outweigh the fixed header cost. See the [explanation](explanation.md) for more details.
