#ifndef PUFFERFISH_STATISTICS_HPP
#define PUFFERFISH_STATISTICS_HPP

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <cmath>
#include <iomanip>
#include <algorithm>
#include "huffman.hpp"

namespace pufferfish {

    struct AnalysisResult {
        std::string filename;
        uint64_t total_symbols  = 0;
        uint16_t unique_symbols = 0;
        int tree_height         = 0;
        int total_nodes         = 0;
        double entropy          = 0.0;
        double avg_code_length  = 0.0;
        double efficiency       = 0.0;

        struct SymbolInfo {
            uint8_t     symbol;
            uint64_t    frequency;
            double      percentage;
            std::string code;
        };
        std::vector<SymbolInfo> top_symbols;
    };

    class Statistics {
    public:
        static AnalysisResult analyze(const std::filesystem::path& file_path);
        static void print_report(const AnalysisResult& result);
    };

} // namespace pufferfish

#endif // PUFFERFISH_STATISTICS_HPP
