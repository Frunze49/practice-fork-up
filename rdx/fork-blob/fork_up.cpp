#include <aws/core/Aws.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <fstream>
#include <iostream>
#include <vector>
#include <openssl/sha.h>

const size_t CHUNK_SIZE = 64 * 1024;  // 64 KB

std::string compute_sha256(const std::vector<char>& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(data.data()), data.size(), hash);

    std::ostringstream hash_string;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        hash_string << std::hex << std::setw(2) << std::setfill('0') << (int)hash[i];
    }
    return hash_string.str();
}

bool check_chunk_exists(Aws::S3::S3Client& s3_client, const std::string& bucket, const std::string& chunk_hash) {
    Aws::S3::Model::HeadObjectRequest head_request;
    head_request.SetBucket(bucket.c_str());
    head_request.SetKey(chunk_hash.c_str());

    auto outcome = s3_client.HeadObject(head_request);
    return outcome.IsSuccess();
}

void upload_chunk(Aws::S3::S3Client& s3_client, const std::string& bucket, const std::string& chunk_hash, const std::vector<char>& data) {
    Aws::S3::Model::PutObjectRequest put_request;
    put_request.SetBucket(bucket.c_str());
    put_request.SetKey(chunk_hash.c_str());

    auto stream = std::make_shared<Aws::StringStream>();
    stream->write(data.data(), data.size());
    put_request.SetBody(stream);

    auto outcome = s3_client.PutObject(put_request);
    if (!outcome.IsSuccess()) {
        std::cerr << "Failed to upload chunk: " << chunk_hash << std::endl;
    } else {
        std::cout << "Uploaded chunk: " << chunk_hash << std::endl;
    }
}

void fork_up(const std::string& file_path, const std::string& bucket) {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    {
        Aws::S3::S3Client s3_client;

        std::ifstream file(file_path, std::ios::binary);
        if (!file) {
            std::cerr << "Error opening file: " << file_path << std::endl;
            return;
        }

        std::vector<char> buffer(CHUNK_SIZE);
        while (file.read(buffer.data(), CHUNK_SIZE) || file.gcount() > 0) {
            size_t bytes_read = file.gcount();
            buffer.resize(bytes_read);

            std::string chunk_hash = compute_sha256(buffer);
            if (!check_chunk_exists(s3_client, bucket, chunk_hash)) {
                upload_chunk(s3_client, bucket, chunk_hash, buffer);
            } else {
                std::cout << "Chunk already exists: " << chunk_hash << std::endl;
            }

            buffer.resize(CHUNK_SIZE);  // Reset buffer size
        }
    }
    Aws::ShutdownAPI(options);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: fork-up <file_path> <s3_bucket>" << std::endl;
        return 1;
    }

    std::string file_path = argv[1];
    std::string bucket = argv[2];

    fork_up(file_path, bucket);
    return 0;
}