#include "archive.hpp"

namespace pufferfish {

    namespace fs = std::filesystem;

    inline constexpr char    PUFF_MAGIC[4]      = {'P', 'U', 'F', 'F'};
    inline constexpr uint8_t PUFF_VERSION_MAJOR = 0x01;
    inline constexpr uint8_t PUFF_VERSION_MINOR = 0x00;

    // ── Helpers ─────────────────────────────────────────────────────────────────

    static void write_u8(std::ostream& out, uint8_t v) { out.put(static_cast<char>(v)); }
    static void write_u16(std::ostream& out, uint16_t v) {
        out.put(static_cast<char>(v & 0xFF));
        out.put(static_cast<char>((v >> 8) & 0xFF));
    }
    static void write_u64(std::ostream& out, uint64_t v) {
        for (int i = 0; i < 8; ++i) out.put(static_cast<char>((v >> (8 * i)) & 0xFF));
    }
    static void write_bytes(std::ostream& out, const char* data, size_t len) {
        out.write(data, static_cast<std::streamsize>(len));
    }

    static uint8_t read_u8(std::istream& in) {
        char ch;
        if (!in.get(ch)) throw std::runtime_error("Unexpected EOF");
        return static_cast<uint8_t>(ch);
    }
    static uint16_t read_u16(std::istream& in) {
        uint16_t v = 0;
        for (int i = 0; i < 2; ++i) v |= static_cast<uint16_t>(read_u8(in)) << (8 * i);
        return v;
    }
    static uint64_t read_u64(std::istream& in) {
        uint64_t v = 0;
        for (int i = 0; i < 8; ++i) v |= static_cast<uint64_t>(read_u8(in)) << (8 * i);
        return v;
    }

    // ── ArchiveWriter ───────────────────────────────────────────────────────────

    void ArchiveWriter::compress(const fs::path& file_path) {
        if (!fs::exists(file_path) || !fs::is_regular_file(file_path)) {
            throw std::runtime_error("Not a regular file: " + file_path.string());
        }

        std::ifstream in(file_path, std::ios::binary);
        if (!in) throw std::runtime_error("Cannot open file: " + file_path.string());

        std::ostringstream raw_buf;
        raw_buf << in.rdbuf();
        std::string raw_data = raw_buf.str();

        if (raw_data.empty()) throw std::runtime_error("File is empty");

        std::istringstream freq_stream(raw_data);
        ByteFrequencyMap frequencies = Encoder::calculate_frequencies(freq_stream);

        HuffmanTree tree;
        tree.build(frequencies);
        HuffmanCodeMap codes = tree.generate_codes();

        std::istringstream encode_stream(raw_data);
        std::ostringstream compressed_buf;
        EncodeResult result = Encoder::encode(encode_stream, compressed_buf, codes);
        std::string compressed_data = compressed_buf.str();

        fs::path output_path = file_path;
        output_path.replace_extension(".puff");

        std::ofstream out(output_path, std::ios::binary);
        if (!out) throw std::runtime_error("Cannot create file: " + output_path.string());

        write_bytes(out, PUFF_MAGIC, 4);
        write_u8(out, PUFF_VERSION_MAJOR);
        write_u8(out, PUFF_VERSION_MINOR);

        std::string filename = file_path.filename().string();
        write_u16(out, static_cast<uint16_t>(filename.size()));
        write_bytes(out, filename.data(), filename.size());

        write_u64(out, raw_data.size());

        write_u16(out, static_cast<uint16_t>(frequencies.size()));
        for (const auto& [sym, freq] : frequencies) {
            write_u8(out, sym);
            write_u64(out, freq);
        }

        write_u8(out, result.padding_bits);
        write_bytes(out, compressed_data.data(), compressed_data.size());
        out.flush();

        double ratio = (1.0 - static_cast<double>(compressed_data.size()) /
                            static_cast<double>(raw_data.size())) * 100.0;

        std::cout << "Compressed: " << file_path.filename().string()
                << " -> " << output_path.filename().string() << "\n"
                << "  Original:   " << raw_data.size() << " bytes\n"
                << "  Compressed: " << compressed_data.size() << " bytes\n"
                << "  Ratio:      " << std::fixed << std::setprecision(1)
                << ratio << "% reduction\n";
    }

    // ── ArchiveReader ───────────────────────────────────────────────────────────

    void ArchiveReader::extract(const fs::path& archive_path) {
        std::ifstream in(archive_path, std::ios::binary);
        if (!in) throw std::runtime_error("Cannot open archive");

        char magic[4]{};
        if (!in.read(magic, 4) || std::memcmp(magic, PUFF_MAGIC, 4) != 0) {
            throw std::runtime_error("Invalid archive format");
        }

        uint8_t major = read_u8(in);
        uint8_t minor = read_u8(in);
        (void)minor;

        if (major != PUFF_VERSION_MAJOR) {
            throw std::runtime_error("Unsupported archive version");
        }

        uint16_t name_len = read_u16(in);
        std::string original_filename(name_len, '\0');
        in.read(original_filename.data(), name_len);

        uint64_t original_size = read_u64(in);

        uint16_t unique_symbols = read_u16(in);
        ByteFrequencyMap frequencies;
        for (uint16_t i = 0; i < unique_symbols; ++i) {
            uint8_t  sym  = read_u8(in);
            uint64_t freq = read_u64(in);
            frequencies[sym] = freq;
        }

        read_u8(in); // padding_bits

        HuffmanTree tree;
        tree.build(frequencies);

        std::ostringstream rest;
        rest << in.rdbuf();
        std::string compressed_data = rest.str();

        fs::path output_path = archive_path.parent_path() / original_filename;
        std::istringstream compressed_stream(compressed_data);
        std::ofstream out(output_path, std::ios::binary);
        if (!out) throw std::runtime_error("Cannot create output file");

        Decoder::decode(compressed_stream, out, tree, original_size);

        std::cout << "Extracted: " << archive_path.filename().string()
                << " -> " << original_filename << "\n"
                << "  Size: " << original_size << " bytes\n";
    }

} // namespace pufferfish
