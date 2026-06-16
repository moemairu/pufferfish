#ifndef PUFFERFISH_ARCHIVE_HPP
#define PUFFERFISH_ARCHIVE_HPP

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <iomanip>
#include <cstring>
#include "huffman.hpp"

namespace pufferfish {
    class ArchiveWriter {
    public:
        static void compress(const std::filesystem::path& file_path);
    };

    class ArchiveReader {
    public:
        static void extract(const std::filesystem::path& archive_path);
    };

} // namespace pufferfish

#endif // PUFFERFISH_ARCHIVE_HPP
