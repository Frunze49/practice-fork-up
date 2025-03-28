#include <fstream>
#include <optional>
#include <vector>

struct ChunkInfo
{
    uint32_t    chunk_size;
    std::string chunk_hash;
};

class FileReader {
protected:
    std::ifstream file;
public:
    FileReader(const std::string& filePath);
    ~FileReader();
};

class Manifest : public FileReader {
public:
    Manifest(const std::string& filePath) : FileReader(filePath) {}
    std::optional<ChunkInfo> get_next_block();
};

class Blob : public FileReader {
public:
    Blob(const std::string& filePath) : FileReader(filePath) {}
    std::vector<char> read_next_data(uint32_t size);
};
