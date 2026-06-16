#include "statistics.hpp"

namespace pufferfish {

    namespace fs = std::filesystem;

    AnalysisResult Statistics::analyze(const fs::path& file_path) {
        AnalysisResult result;
        result.filename = file_path.filename().string();

        std::ifstream in(file_path, std::ios::binary);
        if (!in) throw std::runtime_error("Cannot open file");

        std::ostringstream buf;
        buf << in.rdbuf();
        std::string raw_data = buf.str();

        if (raw_data.empty()) throw std::runtime_error("File is empty");

        std::istringstream freq_stream(raw_data);
        ByteFrequencyMap frequencies = Encoder::calculate_frequencies(freq_stream);

        HuffmanTree tree;
        tree.build(frequencies);
        HuffmanCodeMap codes = tree.generate_codes();

        result.total_symbols  = raw_data.size();
        result.unique_symbols = static_cast<uint16_t>(frequencies.size());
        result.tree_height    = tree.height();
        result.total_nodes    = tree.node_count();

        double entropy = 0.0;
        for (const auto& [sym, freq] : frequencies) {
            double p = static_cast<double>(freq) / static_cast<double>(result.total_symbols);
            if (p > 0) entropy -= p * std::log2(p);
        }
        result.entropy = entropy;

        double avg_len = 0.0;
        for (const auto& [sym, freq] : frequencies) {
            double p = static_cast<double>(freq) / static_cast<double>(result.total_symbols);
            avg_len += p * static_cast<double>(codes[sym].size());
        }
        result.avg_code_length = avg_len;

        result.efficiency = (result.avg_code_length > 0)
            ? (result.entropy / result.avg_code_length) * 100.0
            : 0.0;

        std::vector<std::pair<uint8_t, uint64_t>> freq_vec(frequencies.begin(), frequencies.end());
        std::sort(freq_vec.begin(), freq_vec.end(),
                [](const auto& a, const auto& b) { return a.second > b.second; });

        size_t top_n = std::min<size_t>(10, freq_vec.size());
        for (size_t i = 0; i < top_n; ++i) {
            AnalysisResult::SymbolInfo info;
            info.symbol     = freq_vec[i].first;
            info.frequency  = freq_vec[i].second;
            info.percentage = static_cast<double>(info.frequency) / result.total_symbols * 100.0;

            for (bool b : codes[info.symbol]) info.code += (b ? '1' : '0');
            result.top_symbols.push_back(std::move(info));
        }

        return result;
    }

    void Statistics::print_report(const AnalysisResult& result) {
        auto format_symbol = [](uint8_t sym) -> std::string {
            if (sym == ' ') return "' '";
            if (sym == '\n') return "'\\n'";
            if (sym >= 33 && sym < 127) return std::string("'") + static_cast<char>(sym) + "'";
            char buf[8];
            std::snprintf(buf, sizeof(buf), "0x%02X", sym);
            return buf;
        };

        auto format_num = [](uint64_t n) {
            std::string s = std::to_string(n);
            int p = static_cast<int>(s.length()) - 3;
            while (p > 0) { s.insert(p, ","); p -= 3; }
            return s;
        };

        std::cout << "\n"
            << "═══════════════════════════════════════════════════\n"
            << "  Pufferfish — Huffman Analysis\n"
            << "═══════════════════════════════════════════════════\n"
            << "  File:                    " << result.filename << "\n"
            << "  Total Symbols:           " << format_num(result.total_symbols) << "\n"
            << "  Unique Symbols:          " << result.unique_symbols << "\n"
            << "───────────────────────────────────────────────────\n"
            << "  Tree Height:             " << result.tree_height << "\n"
            << "  Total Nodes:             " << result.total_nodes << "\n"
            << "───────────────────────────────────────────────────\n"
            << std::fixed << std::setprecision(2)
            << "  Shannon Entropy:         " << result.entropy << " bits/symbol\n"
            << "  Average Code Length:     " << result.avg_code_length << " bits/symbol\n"
            << "  Compression Efficiency:  " << std::setprecision(1) << result.efficiency << "%\n"
            << "───────────────────────────────────────────────────\n"
            << "  Top Symbols:\n";

        for (const auto& sym : result.top_symbols) {
            std::cout << "    " << std::left << std::setw(6) << format_symbol(sym.symbol)
                    << " → " << std::right << std::setw(5) << std::setprecision(1)
                    << sym.percentage << "%   Code: " << sym.code << "\n";
        }
        std::cout << "═══════════════════════════════════════════════════\n\n";
    }

} // namespace pufferfish
