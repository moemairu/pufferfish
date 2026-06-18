#ifndef PUFFERFISH_HUFFMAN_HPP
#define PUFFERFISH_HUFFMAN_HPP

#include <cstdint>
#include <vector>
#include <unordered_map>
#include <memory>
#include <istream>
#include <ostream>
#include <optional>
#include <queue>
#include <stdexcept>
#include <algorithm>

namespace pufferfish {

    using ByteFrequencyMap = std::unordered_map<uint8_t, uint64_t>;
    using HuffmanCodeMap   = std::unordered_map<uint8_t, std::vector<bool>>;

    // ── Node ────────────────────────────────────────────────────────────────────

    struct HuffmanNode {
        uint8_t  symbol    = 0;
        uint64_t frequency = 0;
        std::unique_ptr<HuffmanNode> left;
        std::unique_ptr<HuffmanNode> right;

        HuffmanNode(uint8_t sym, uint64_t freq);
        HuffmanNode(std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r);
        [[nodiscard]] bool is_leaf() const noexcept;
    };

    // ── Tree ────────────────────────────────────────────────────────────────────

    class HuffmanTree {
    public:
        HuffmanTree() = default;
        void build(const ByteFrequencyMap& frequencies);
        [[nodiscard]] HuffmanCodeMap generate_codes() const;
        [[nodiscard]] const HuffmanNode* get_root() const noexcept;
        [[nodiscard]] bool empty() const noexcept;
        [[nodiscard]] int height() const noexcept;
        [[nodiscard]] int node_count() const noexcept;

    private:
        std::unique_ptr<HuffmanNode> root_;
        static void generate_codes_impl(const HuffmanNode* node, std::vector<bool>& current_code, HuffmanCodeMap& codes);
        static int height_impl(const HuffmanNode* node) noexcept;
        static int node_count_impl(const HuffmanNode* node) noexcept;
    };

    // ── Bit IO ──────────────────────────────────────────────────────────────────

    class BitWriter {
    public:
        explicit BitWriter(std::ostream& out);
        void write_bit(bool bit);
        void write_bits(const std::vector<bool>& bits);
        uint8_t flush();
        [[nodiscard]] uint64_t bytes_written() const noexcept;

    private:
        std::ostream& out_;
        uint8_t  buffer_        = 0;
        int      bit_count_     = 0;
        uint64_t bytes_written_ = 0;
    };

    class BitReader {
    public:
        explicit BitReader(std::istream& in);
        std::optional<bool> read_bit();

    private:
        std::istream& in_;
        uint8_t buffer_         = 0;
        int     bits_remaining_ = 0;
    };

    // ── Encoder / Decoder ───────────────────────────────────────────────────────

    struct EncodeResult {
        uint64_t compressed_size = 0;
        uint8_t  padding_bits    = 0;
    };

    class Encoder {
    public:
        static ByteFrequencyMap calculate_frequencies(std::istream& input);
        static EncodeResult encode(std::istream& input, std::ostream& output, const HuffmanCodeMap& codes);
    };

    class Decoder {
    public:
        static void decode(std::istream& input, std::ostream& output, const HuffmanTree& tree, uint64_t original_size);
    };

} // namespace pufferfish

#endif // PUFFERFISH_HUFFMAN_HPP
