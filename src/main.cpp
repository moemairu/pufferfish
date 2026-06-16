#include "archive.hpp"
#include "statistics.hpp"

namespace pufferfish {

    void print_usage(const std::string& program) {
        std::cout <<
        R"(Pufferfish — Educational Huffman Coding Utility

        Usage:
        )" << program << R"( <command> [file]

        Commands:
        compress <file>       Compress a file into a .puff archive
        extract  <archive>    Extract a .puff archive
        analyze  <file>       Show detailed Huffman analysis of a file
        help                  Show this help message

        Examples:
        )" << program << R"( compress novel.txt     →  novel.puff
        )" << program << R"( extract  novel.puff    →  novel.txt
        )" << program << R"( analyze  novel.txt
        )";
    }

} // namespace pufferfish

int main(int argc, char* argv[]) {
    if (argc < 2) {
        pufferfish::print_usage(argv[0]);
        return 1;
    }

    std::string cmd = argv[1];
    if (cmd == "c") cmd = "compress";
    if (cmd == "x") cmd = "extract";
    if (cmd == "a") cmd = "analyze";

    try {
        if (cmd == "compress") {
            if (argc < 3) throw std::runtime_error("Requires file argument");
            pufferfish::ArchiveWriter::compress(argv[2]);
            return 0;
        }
        if (cmd == "extract") {
            if (argc < 3) throw std::runtime_error("Requires archive argument");
            pufferfish::ArchiveReader::extract(argv[2]);
            return 0;
        }
        if (cmd == "analyze") {
            if (argc < 3) throw std::runtime_error("Requires file argument");
            auto res = pufferfish::Statistics::analyze(argv[2]);
            pufferfish::Statistics::print_report(res);
            return 0;
        }
        if (cmd == "help" || cmd == "-h" || cmd == "--help") {
            pufferfish::print_usage(argv[0]);
            return 0;
        }
        throw std::runtime_error("Unknown command: " + cmd);
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
}
