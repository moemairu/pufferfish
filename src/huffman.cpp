#include "huffman.hpp"

namespace pufferfish {

    // ── HuffmanNode ─────────────────────────────────────────────────────────────

    HuffmanNode::HuffmanNode(uint8_t sym, uint64_t freq)
        : symbol(sym), frequency(freq) {}

    HuffmanNode::HuffmanNode(std::unique_ptr<HuffmanNode> l, std::unique_ptr<HuffmanNode> r)
        : symbol(0), frequency(l->frequency + r->frequency),
        left(std::move(l)), right(std::move(r)) {}

    bool HuffmanNode::is_leaf() const noexcept {
        return !left && !right;
    }

    // ── HuffmanTree ─────────────────────────────────────────────────────────────

    void HuffmanTree::build(const ByteFrequencyMap& frequencies) {
        if (frequencies.empty()) {
            root_ = nullptr;
            return;
        }

        auto cmp = [](const std::unique_ptr<HuffmanNode>& a, const std::unique_ptr<HuffmanNode>& b) {
            if (a->frequency != b->frequency) return a->frequency > b->frequency;
            return a->symbol > b->symbol;
        };

        std::priority_queue<std::unique_ptr<HuffmanNode>, std::vector<std::unique_ptr<HuffmanNode>>, decltype(cmp)> pq(cmp);

        // Sort by symbol ascending to ensure deterministic push order.
        // unordered_map iteration order depends on insert order, which
        // differs between compress (from file scan) and extract (from
        // archive header). Without sorting, tie-breaking among internal
        // nodes with equal frequencies becomes non-deterministic, producing
        // different trees for the same frequency data.
        std::vector<std::pair<uint8_t, uint64_t>> sorted_freq(frequencies.begin(), frequencies.end());
        std::sort(sorted_freq.begin(), sorted_freq.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });

        for (const auto& [sym, freq] : sorted_freq) {
            pq.push(std::make_unique<HuffmanNode>(sym, freq));
        }

        while (pq.size() > 1) {
            auto left  = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top())); pq.pop();
            auto right = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top())); pq.pop();
            pq.push(std::make_unique<HuffmanNode>(std::move(left), std::move(right)));
        }

        root_ = std::move(const_cast<std::unique_ptr<HuffmanNode>&>(pq.top()));
        pq.pop();
    }

    HuffmanCodeMap HuffmanTree::generate_codes() const {
        HuffmanCodeMap codes;
        if (!root_) return codes;

        std::vector<bool> current_code;
        if (root_->is_leaf()) {
            codes[root_->symbol] = {false};
            return codes;
        }

        generate_codes_impl(root_.get(), current_code, codes);
        return codes;
    }

    void HuffmanTree::generate_codes_impl(const HuffmanNode* node, std::vector<bool>& current_code, HuffmanCodeMap& codes) {
        if (!node) return;
        if (node->is_leaf()) {
            codes[node->symbol] = current_code;
            return;
        }
        current_code.push_back(false);
        generate_codes_impl(node->left.get(), current_code, codes);
        current_code.pop_back();

        current_code.push_back(true);
        generate_codes_impl(node->right.get(), current_code, codes);
        current_code.pop_back();
    }

    const HuffmanNode* HuffmanTree::get_root() const noexcept { return root_.get(); }
    bool HuffmanTree::empty() const noexcept { return root_ == nullptr; }
    int HuffmanTree::height() const noexcept { return height_impl(root_.get()); }
    int HuffmanTree::node_count() const noexcept { return node_count_impl(root_.get()); }

    int HuffmanTree::height_impl(const HuffmanNode* node) noexcept {
        if (!node) return 0;
        if (node->is_leaf()) return 1;
        return 1 + std::max(height_impl(node->left.get()), height_impl(node->right.get()));
    }

    int HuffmanTree::node_count_impl(const HuffmanNode* node) noexcept {
        if (!node) return 0;
        return 1 + node_count_impl(node->left.get()) + node_count_impl(node->right.get());
    }

    // ── BitWriter ───────────────────────────────────────────────────────────────

    BitWriter::BitWriter(std::ostream& out) : out_(out) {}

    void BitWriter::write_bit(bool bit) {
        buffer_ = static_cast<uint8_t>((buffer_ << 1) | (bit ? 1 : 0));
        ++bit_count_;
        if (bit_count_ == 8) {
            out_.put(static_cast<char>(buffer_));
            ++bytes_written_;
            buffer_    = 0;
            bit_count_ = 0;
        }
    }

    void BitWriter::write_bits(const std::vector<bool>& bits) {
        for (bool b : bits) write_bit(b);
    }

    uint8_t BitWriter::flush() {
        if (bit_count_ == 0) return 0;
        uint8_t padding = static_cast<uint8_t>(8 - bit_count_);
        buffer_ = static_cast<uint8_t>(buffer_ << padding);
        out_.put(static_cast<char>(buffer_));
        ++bytes_written_;
        buffer_    = 0;
        bit_count_ = 0;
        return padding;
    }

    uint64_t BitWriter::bytes_written() const noexcept { return bytes_written_; }

    // ── BitReader ───────────────────────────────────────────────────────────────

    BitReader::BitReader(std::istream& in) : in_(in) {}

    std::optional<bool> BitReader::read_bit() {
        if (bits_remaining_ == 0) {
            char ch;
            if (!in_.get(ch)) return std::nullopt;
            buffer_         = static_cast<uint8_t>(ch);
            bits_remaining_ = 8;
        }
        --bits_remaining_;
        return (buffer_ >> bits_remaining_) & 1;
    }

    // ── Encoder ─────────────────────────────────────────────────────────────────

    ByteFrequencyMap Encoder::calculate_frequencies(std::istream& input) {
        ByteFrequencyMap freq;
        char ch;
        while (input.get(ch)) {
            ++freq[static_cast<uint8_t>(ch)];
        }
        return freq;
    }

    EncodeResult Encoder::encode(std::istream& input, std::ostream& output, const HuffmanCodeMap& codes) {
        BitWriter writer(output);
        char ch;
        while (input.get(ch)) {
            auto it = codes.find(static_cast<uint8_t>(ch));
            if (it != codes.end()) {
                writer.write_bits(it->second);
            }
        }
        uint8_t padding = writer.flush();
        return {writer.bytes_written(), padding};
    }

    // ── Decoder ─────────────────────────────────────────────────────────────────

    void Decoder::decode(std::istream& input, std::ostream& output, const HuffmanTree& tree, uint64_t original_size) {
        if (original_size == 0 || tree.empty()) return;

        const HuffmanNode* root = tree.get_root();
        BitReader reader(input);

        if (root->is_leaf()) {
            for (uint64_t i = 0; i < original_size; ++i) {
                output.put(static_cast<char>(root->symbol));
                reader.read_bit();
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
            current = bit.value() ? current->right.get() : current->left.get();
            if (!current) {
                throw std::runtime_error("Invalid Huffman tree path");
            }
            if (current->is_leaf()) {
                output.put(static_cast<char>(current->symbol));
                ++decoded;
                current = root;
            }
        }
    }

} // namespace pufferfish
