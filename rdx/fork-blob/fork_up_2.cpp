#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

// Structure to represent a chunk in the manifest
struct Chunk {
    size_t size;
    std::string hash;
    size_t offset;
};

// Global mutex for thread-safe console output
std::mutex cout_mutex;

void log(const std::string& message) {
    std::lock_guard<std::mutex> lock(cout_mutex);
    std::cout << message << std::endl;
}

// Parse the RDX manifest file
std::vector<Chunk> parseManifest(const std::string& manifestPath) {
    std::vector<Chunk> chunks;
    std::ifstream file(manifestPath);
    if (!file) {
        throw std::runtime_error("Cannot open manifest file: " + manifestPath);
    }

    json manifest;
    try {
        file >> manifest;
    } catch (const json::parse_error& e) {
        throw std::runtime_error("Failed to parse manifest JSON: " + std::string(e.what()));
    }

    if (!manifest.is_array()) {
        throw std::runtime_error("Manifest is not a JSON array");
    }

    size_t offset = 0;
    for (const auto& item : manifest) {
        if (!item.is_string()) {
            throw std::runtime_error("Manifest entry is not a string");
        }

        std::string entry = item.get<std::string>();
        size_t colonPos = entry.find(':');
        if (colonPos == std::string::npos) {
            throw std::runtime_error("Invalid manifest entry format (missing colon): " + entry);
        }

        try {
            Chunk chunk;
            chunk.size = std::stoul(entry.substr(0, colonPos));
            chunk.hash = entry.substr(colonPos + 1);
            chunk.offset = offset;
            chunks.push_back(chunk);
            offset += chunk.size;
        } catch (const std::exception& e) {
            throw std::runtime_error("Invalid chunk size in manifest entry: " + entry);
        }
    }

    return chunks;
}

// Check if a chunk already exists in the S3 bucket
bool chunkExists(const Aws::S3::S3Client& s3Client, const std::string& bucketName, const std::string& chunkHash) {
    Aws::S3::Model::HeadObjectRequest request;
    request.SetBucket(bucketName);
    request.SetKey(chunkHash);

    auto outcome = s3Client.HeadObject(request);
    return outcome.IsSuccess();
}

// Upload a chunk to the S3 bucket
void uploadChunk(const Aws::S3::S3Client& s3Client, const std::string& bucketName,
                const std::string& chunkHash, const std::vector<unsigned char>& chunkData) {
    Aws::S3::Model::PutObjectRequest request;
    request.SetBucket(bucketName);
    request.SetKey(chunkHash);
    
    auto bodyStream = Aws::MakeShared<Aws::StringStream>("PutObjectInputStream");
    bodyStream->write(reinterpret_cast<const char*>(chunkData.data()), chunkData.size());
    request.SetBody(bodyStream);

    auto outcome = s3Client.PutObject(request);
    if (!outcome.IsSuccess()) {
        log("Error uploading chunk " + chunkHash + ": " + outcome.GetError().GetMessage());
        throw std::runtime_error("Failed to upload chunk: " + chunkHash);
    } else {
        log("Uploaded chunk: " + chunkHash);
    }
}

// Process a single chunk
void processChunk(const Aws::S3::S3Client& s3Client, const std::string& bucketName, 
                  const std::string& blobPath, const Chunk& chunk,
                  std::atomic<size_t>& uploadedCount, std::atomic<size_t>& existingCount) {
    try {
        // Check if chunk already exists in S3
        if (!chunkExists(s3Client, bucketName, chunk.hash)) {
            // Read chunk from blob
            std::ifstream blobFile(blobPath, std::ios::binary);
            if (!blobFile) {
                throw std::runtime_error("Cannot open blob file: " + blobPath);
            }
            
            std::vector<unsigned char> chunkData(chunk.size);
            blobFile.seekg(chunk.offset);
            blobFile.read(reinterpret_cast<char*>(chunkData.data()), chunk.size);
            
            if (blobFile.gcount() != static_cast<std::streamsize>(chunk.size)) {
                throw std::runtime_error("Failed to read full chunk from blob");
            }
            
            // Upload chunk
            uploadChunk(s3Client, bucketName, chunk.hash, chunkData);
            uploadedCount++;
        } else {
            log("Chunk already exists: " + chunk.hash);
            existingCount++;
        }
    } catch (const std::exception& e) {
        log("Error processing chunk " + chunk.hash + ": " + e.what());
        throw; // Re-throw to be caught by main thread
    }
}

int main(int argc, char* argv[]) {
    if (argc < 7) {
        std::cerr << "Usage: " << argv[0] << " <manifest_path> <blob_path> <s3_endpoint> <access_key> <secret_key> <bucket_name> [threads]\n";
        return 1;
    }

    std::string manifestPath = argv[1];
    std::string blobPath = argv[2];
    std::string s3Endpoint = argv[3];
    std::string accessKey = argv[4];
    std::string secretKey = argv[5];
    std::string bucketName = argv[6];
    
    size_t threadCount = (argc > 7) ? std::stoul(argv[7]) : std::thread::hardware_concurrency();
    if (threadCount == 0) threadCount = 1;
    
    log("Starting fork-up");
    log("Manifest: " + manifestPath);
    log("Blob: " + blobPath);
    log("S3 Endpoint: " + s3Endpoint);
    log("Bucket: " + bucketName);
    log("Using " + std::to_string(threadCount) + " threads");

    // Initialize AWS SDK
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        // Configure S3 client
        Aws::Client::ClientConfiguration clientConfig;
        clientConfig.endpointOverride = s3Endpoint;
        clientConfig.scheme = Aws::Http::Scheme::HTTP; // Change to HTTPS if needed
        clientConfig.connectTimeoutMs = 30000; // 30 seconds
        clientConfig.requestTimeoutMs = 60000; // 60 seconds
        
        Aws::Auth::AWSCredentials credentials(accessKey, secretKey);
        Aws::S3::S3Client s3Client(credentials, clientConfig);

        try {
            // Parse manifest
            std::vector<Chunk> chunks = parseManifest(manifestPath);
            log("Parsed manifest with " + std::to_string(chunks.size()) + " chunks");

            // Set up stats counters
            std::atomic<size_t> uploadedCount(0);
            std::atomic<size_t> existingCount(0);
            
            // Process chunks in parallel
            std::vector<std::thread> threads;
            std::vector<std::exception_ptr> exceptions(threadCount);
            
            size_t chunksPerThread = (chunks.size() + threadCount - 1) / threadCount;
            
            for (size_t t = 0; t < threadCount; ++t) {
                threads.emplace_back([&, t]() {
                    try {
                        size_t start = t * chunksPerThread;
                        size_t end = std::min(start + chunksPerThread, chunks.size());
                        
                        for (size_t i = start; i < end; ++i) {
                            processChunk(s3Client, bucketName, blobPath, chunks[i], uploadedCount, existingCount);
                        }
                    } catch (...) {
                        exceptions[t] = std::current_exception();
                    }
                });
            }
            
            // Wait for all threads to complete
            for (auto& thread : threads) {
                if (thread.joinable()) {
                    thread.join();
                }
            }
            
            // Check for exceptions
            for (const auto& e : exceptions) {
                if (e) {
                    std::rethrow_exception(e);
                }
            }
            
            log("Upload summary:");
            log("  Total chunks: " + std::to_string(chunks.size()));
            log("  Uploaded: " + std::to_string(uploadedCount));
            log("  Already existed: " + std::to_string(existingCount));
            
            log("All chunks processed successfully.");
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            Aws::ShutdownAPI(options);
            return 1;
        }
    }
    
    Aws::ShutdownAPI(options);
    return 0;
}
