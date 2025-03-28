#include "helper.hpp"
#include <iostream>

FileReader::FileReader(const std::string& filePath)
{
    file.open(filePath);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filePath);
    }
}

FileReader::~FileReader() {
    if (file.is_open()) {
        file.close();
    }
}

std::optional<ChunkInfo> Manifest::GetNextBlock() {
    std::string line;
    if (std::getline(file, line)) {
        size_t delimiter_pos = line.find(':');
        if (delimiter_pos != std::string::npos) {
            try {
                ChunkInfo chunk_info;
                chunk_info.chunk_size = std::stoul(line.substr(0, delimiter_pos));
                chunk_info.chunk_hash = line.substr(delimiter_pos + 1);
                if (chunk_info.chunk_hash.empty()) {
                    return std::nullopt;
                }
                return chunk_info;
            } catch (const std::exception& e) {
                std::cerr << "Error parsing line: " << line << " - " << e.what() << std::endl;
            }
        }
    }
    return std::nullopt;
}


std::vector<char> Blob::ReadNextData(uint32_t size) {
    std::vector<char> buffer(size);
    file.read(reinterpret_cast<char*>(buffer.data()), size);
    std::streamsize bytes_read = file.gcount();
    if (bytes_read < size) {
        throw std::runtime_error("Fork-blob works bad!");
    }
    return buffer;
}