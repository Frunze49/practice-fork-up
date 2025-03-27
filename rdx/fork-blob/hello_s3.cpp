#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListBucketsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <iostream>

Aws::S3::S3Client create_minio_client() {
    // Set up the client configuration
    Aws::Client::ClientConfiguration config;
    
    // Configure for MinIO
    config.endpointOverride = "localhost:9002";  // MinIO server address
    config.scheme = Aws::Http::Scheme::HTTP;     // Use HTTPS if MinIO is configured with TLS
    config.verifySSL = false;                    // Set to true if using proper certificates
    
    // Create and return the S3 client with MinIO-specific settings
    return Aws::S3::S3Client(Aws::Auth::AWSCredentials("admin", "password123"), config, 
                             Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
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

int main() {
    Aws::SDKOptions options;
    Aws::InitAPI(options);
    
    const std::string bucket = "my-bucket";
    const std::string hash = "f976ac19c16949fc098966a870332a3865f3783d3c350de20fbd710aa9fcc87b";
    std::string str = "BALDEZH";
    std::vector<char> data(str.begin(), str.end()); 

    try {
        // Create the MinIO client
        Aws::S3::S3Client s3Client = create_minio_client();
        std::cout << "Connection is created." << std::endl;
        // Test the connection by listing buckets
        auto outcome = s3Client.ListBuckets();
        if (outcome.IsSuccess()) {
            std::cout << "Connected to MinIO successfully. Buckets:" << std::endl;
            for (auto&& b : outcome.GetResult().GetBuckets()) {
                std::cout << "  " << b.GetName() << std::endl;
            }
        } else {
            std::cerr << "Error: " << outcome.GetError().GetMessage() << std::endl;
        }

        if (!check_chunk_exists(s3Client, bucket, hash)) {
            upload_chunk(s3Client, bucket, hash, data);
        }

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
    
    Aws::ShutdownAPI(options);
    return 0;
}