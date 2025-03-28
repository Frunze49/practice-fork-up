// test_utils.cpp
#include <fstream>
#include <random>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

void GenerateBinaryFile(const std::string& path, size_t size_bytes) {
    std::ofstream file(path, std::ios::binary);
    std::vector<char> buffer(4096);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(0, 255);
    
    size_t remaining = size_bytes;
    while (remaining > 0) {
        size_t chunk_size = std::min(remaining, buffer.size());
        for (size_t i = 0; i < chunk_size; ++i) {
            buffer[i] = static_cast<char>(distrib(gen));
        }
        file.write(buffer.data(), chunk_size);
        remaining -= chunk_size;
    }
}

void CreateModifiedVersion(const std::string& source, const std::string& dest, 
                           float change_ratio, size_t offset = 0) {
    std::ifstream src(source, std::ios::binary);
    std::ofstream dst(dest, std::ios::binary);
    
    src.seekg(0, std::ios::end);
    size_t file_size = src.tellg();
    src.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> data(file_size);
    src.read(reinterpret_cast<char*>(data.data()), file_size);
    
    // Изменяем указанный процент байтов, начиная с offset
    size_t changes = static_cast<size_t>(file_size * change_ratio);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> val_distrib(0, 255);
    
    for (size_t i = 0; i < changes; ++i) {
        size_t pos = (offset + i) % file_size;
        data[pos] = val_distrib(gen);
    }
    
    dst.write(reinterpret_cast<char*>(data.data()), file_size);
}

void GenerateManifest(const std::string& file_path, const std::string& manifest_path) {
    std::ifstream input(file_path, std::ios::binary);
    std::ofstream manifest(manifest_path);
    
    if (!input || !manifest) {
        std::cerr << "Error opening files!" << std::endl;
        return;
    }
    
    input.seekg(0, std::ios::end);
    size_t file_size = input.tellg();
    input.seekg(0, std::ios::beg);
    
    const size_t target_chunk_size = 64 * 1024;
    std::vector<char> buffer(target_chunk_size);
    
    manifest << "[\n";
    
    std::random_device rd;
    std::mt19937 gen(rd());
    
    size_t remaining = file_size;
    bool first_chunk = true;
    
    while (remaining > 0) {
        std::uniform_int_distribution<> chunk_size_dist(
            target_chunk_size * 0.8, target_chunk_size * 1.2);
        size_t chunk_size = std::min(remaining, static_cast<size_t>(chunk_size_dist(gen)));
        
        input.read(buffer.data(), chunk_size);
        
        std::stringstream hash;
        for (int i = 0; i < 64; i++) {
            hash << std::hex << (gen() % 16);
        }
        
        if (!first_chunk) manifest << "\n";
        manifest << "    " << chunk_size << ":" << hash.str();
        
        first_chunk = false;
        remaining -= chunk_size;
    }
    
    manifest << "\n]\n";
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <command> <args...>\n";
        std::cerr << "Commands:\n";
        std::cerr << "  generate <filename> <size_bytes>\n";
        std::cerr << "  modify <source> <dest> <change_ratio> [offset]\n";
        std::cerr << "  manifest <file_path> <manifest_path>\n";
        return 1;
    }
    
    std::string cmd = argv[1];
    
    if (cmd == "generate") {
        if (argc < 4) return 1;
        GenerateBinaryFile(argv[2], std::stoul(argv[3]));
    } 
    else if (cmd == "modify") {
        if (argc < 5) return 1;
        size_t offset = argc > 5 ? std::stoul(argv[5]) : 0;
        CreateModifiedVersion(argv[2], argv[3], std::stof(argv[4]), offset);
    }
    else if (cmd == "manifest") {
        if (argc < 4) return 1;
        GenerateManifest(argv[2], argv[3]);
    }
    
    return 0;
}
