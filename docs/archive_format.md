# .puff Archive Format

This document describes the binary layout of `.puff` files produced by Pufferfish.

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
| 8 | Filename | N bytes | Original filename (UTF-8, e.g. `novel.txt`) |
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
09 00                        length = 9
6E 6F 76 65 6C 2E 74 78 74  "novel.txt"
```

### Original File Size (8 bytes)

The size of the uncompressed file. The decoder uses this to know when to stop — without it, the padding bits at the end of the bitstream would be misinterpreted as data.

### Frequency Table (2 + 9×N bytes)

The number of unique symbols, followed by pairs of (symbol, frequency):

```
03 00          3 unique symbols
61 05 00 00 00 00 00 00 00   'a' appears 5 times
62 02 00 00 00 00 00 00 00   'b' appears 2 times
63 01 00 00 00 00 00 00 00   'c' appears 1 time
```

The reader uses this table to rebuild the Huffman tree. Because tree construction is deterministic (same comparator, same tie-breaking), the rebuilt tree is identical to the one used during compression.

### Padding Bits (1 byte)

The number of zero bits padded at the end of the last byte of encoded data (0–7). This is necessary because Huffman codes are variable-length and the total number of bits may not be a multiple of 8.

### Encoded Data (remaining bytes)

The Huffman-encoded bitstream. Bits are packed MSB-first (most significant bit first).

## Example

Compressing `"hello"` (5 bytes):

```
Frequencies: h=1, e=1, l=2, o=1
Codes:       l=0, h=10, e=110, o=111

Encoded: h    e     l  l  o
         10   110   0  0  111
         = 10110001 11______    (6 padding bits)
```

The resulting `.puff` file:
```
50 55 46 46          PUFF magic
01 00                v1.0
05 00                filename length = 5
68 65 6C 6C 6F       "hello" (filename — this is the name, not the content)
05 00 00 00 00 00 00 00   original size = 5
04 00                4 unique symbols
68 01 00 00 00 00 00 00 00   'h' = 1
65 01 00 00 00 00 00 00 00   'e' = 1
6C 02 00 00 00 00 00 00 00   'l' = 2
6F 01 00 00 00 00 00 00 00   'o' = 1
06                   6 padding bits
B1 C0                encoded data (10110001 11000000)
```
