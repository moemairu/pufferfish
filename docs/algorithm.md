# Huffman Coding Algorithm

This document explains the Huffman coding algorithm as implemented in Pufferfish.

## Overview

Huffman coding is a **lossless compression** algorithm invented by David Huffman in 1952. It assigns **variable-length binary codes** to symbols based on their frequencies: frequent symbols get short codes, rare symbols get long codes.

The algorithm is **greedy** — at each step, it makes the locally optimal choice (merging the two least frequent nodes), which leads to a globally optimal solution.

## Step 1: Frequency Analysis

Read the input and count how many times each byte value appears.

**Example input:** `"abracadabra"`

| Symbol | Frequency |
|--------|-----------|
| `a`    | 5         |
| `b`    | 2         |
| `r`    | 2         |
| `c`    | 1         |
| `d`    | 1         |

## Step 2: Build the Huffman Tree

Use a **priority queue (min-heap)** to construct a binary tree:

1. Create a leaf node for each symbol with its frequency.
2. Insert all leaves into a min-heap (smallest frequency = highest priority).
3. While the heap has more than one node:
   - Extract the two nodes with the smallest frequencies.
   - Create a new internal node with these two as children and frequency = sum.
   - Insert the new node back into the heap.
4. The last remaining node is the **root** of the Huffman tree.

**Construction trace for `"abracadabra"`:**

```
Initial heap: [c:1] [d:1] [b:2] [r:2] [a:5]

Merge c(1) + d(1) → cd(2)
Heap: [b:2] [r:2] [cd:2] [a:5]

Merge b(2) + r(2) → br(4)
Heap: [cd:2] [br:4] [a:5]

Merge cd(2) + br(4) → cdbr(6)
Heap: [a:5] [cdbr:6]

Merge a(5) + cdbr(6) → root(11)
Heap: [root:11]   ← done
```

**Resulting tree:**

```
        [root:11]
        /        \
     [a:5]    [cdbr:6]
              /       \
          [cd:2]    [br:4]
          /    \    /    \
       [c:1] [d:1][b:2] [r:2]
```

### Why is this greedy?

At each step, we merge the two **cheapest** nodes — the ones with the lowest frequency. This greedy choice ensures that high-frequency symbols end up near the root (getting short codes) and low-frequency symbols end up deeper (getting longer codes).

### Properties of the tree

- It is a **full binary tree**: every internal node has exactly two children.
- With `n` unique symbols, the tree has `2n - 1` total nodes.
- The tree **height** equals the longest code length.

## Step 3: Generate Codes

Traverse the tree recursively:
- Going **left** → append `0`
- Going **right** → append `1`
- Reaching a **leaf** → the accumulated bits are that symbol's code

**Generated codes for `"abracadabra"`:**

| Symbol | Code  | Length |
|--------|-------|--------|
| `a`    | `0`   | 1 bit  |
| `c`    | `100` | 3 bits |
| `d`    | `101` | 3 bits |
| `b`    | `110` | 3 bits |
| `r`    | `111` | 3 bits |

### The Prefix Property

No code is a prefix of any other code. For example, `0` (the code for `a`) is not a prefix of `100`, `101`, `110`, or `111`. This is guaranteed by the tree structure — codes only exist at leaves, and no leaf is an ancestor of another leaf.

This property makes decoding **unambiguous**: there is exactly one way to parse a Huffman-encoded bitstream.

## Step 4: Encoding

Replace each input byte with its Huffman code:

```
Input:   a    b    r    a    c    a    d    a    b    r    a
Codes:   0    110  111  0    100  0    101  0    110  111  0
```

**Encoded bitstream:** `0 110 111 0 100 0 101 0 110 111 0`

That's **23 bits** instead of the original **88 bits** (11 bytes × 8 bits), a compression ratio of **73.9%**.

The bits are packed into bytes for storage. If the total number of bits is not a multiple of 8, the last byte is padded with zeros and the padding count is stored in the archive.

## Step 5: Decoding

Start at the root of the Huffman tree. For each bit in the stream:
- `0` → go **left**
- `1` → go **right**
- When you reach a **leaf** → output that symbol, go back to the root

**Decoding the first few bits:**

```
Bits: 0 1 1 0 1 1 1 0 ...
          
Bit 0 → root → left → leaf 'a' → output 'a', restart
Bit 1 → root → right
Bit 1 →      → right
Bit 0 →      → left → leaf 'b' → output 'b', restart
Bit 1 → root → right
Bit 1 →      → right
Bit 1 →      → right → leaf 'r' → output 'r', restart
Bit 0 → root → left → leaf 'a' → output 'a', restart
...
```

We stop after decoding the expected number of bytes (original file size), which avoids interpreting the padding bits at the end.

## Information Theory

### Shannon Entropy

The **Shannon entropy** of a source is defined as:

```
H = -Σ p(x) × log₂(p(x))
```

where `p(x)` is the probability of symbol `x`. This gives the **theoretical minimum** average bits per symbol — no coding scheme can do better.

For `"abracadabra"`:
- `p(a) = 5/11 ≈ 0.455`
- `p(b) = 2/11 ≈ 0.182`
- `p(r) = 2/11 ≈ 0.182`
- `p(c) = 1/11 ≈ 0.091`
- `p(d) = 1/11 ≈ 0.091`

```
H ≈ -(0.455 × log₂(0.455) + 2 × 0.182 × log₂(0.182) + 2 × 0.091 × log₂(0.091))
H ≈ 2.04 bits/symbol
```

### Average Code Length

The **average code length** of a Huffman code is:

```
L = Σ p(x) × len(code(x))
```

For our example:
```
L = 0.455 × 1 + 0.182 × 3 + 0.182 × 3 + 0.091 × 3 + 0.091 × 3
L = 0.455 + 0.545 + 0.545 + 0.273 + 0.273
L ≈ 2.09 bits/symbol
```

### Compression Efficiency

```
η = (H / L) × 100%
η ≈ (2.04 / 2.09) × 100%
η ≈ 97.6%
```

Huffman coding guarantees: **H ≤ L < H + 1**. The average code length is always within 1 bit of the entropy, making Huffman coding near-optimal for symbol-by-symbol encoding.
