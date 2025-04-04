#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/ListBucketsRequest.h>
#include <aws/s3/model/PutObjectRequest.h>
#include <aws/s3/model/HeadObjectRequest.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/s3/model/GetObjectRequest.h>
#include <iostream>
#include "client.hpp"

S3Client::S3Client(const S3Config& s3_config) {
    Aws::Client::ClientConfiguration client_config;

    client_config.region = "us-east-1";
    client_config.endpointOverride = s3_config.endpoint;
    client_config.scheme = Aws::Http::Scheme::HTTP;
    client_config.verifySSL = false;
    
    s3_client = Aws::S3::S3Client(Aws::Auth::AWSCredentials(s3_config.access_key, s3_config.secret_key), client_config, 
                             Aws::Client::AWSAuthV4Signer::PayloadSigningPolicy::Never, false);
}

bool S3Client::CheckChunkExists(const std::string& bucket, const std::string& chunk_hash) const {
    Aws::S3::Model::HeadObjectRequest head_request;
    head_request.SetBucket(bucket.c_str());
    head_request.SetKey(chunk_hash.c_str());

    auto outcome = s3_client.HeadObject(head_request);
    return outcome.IsSuccess();
}

bool S3Client::CheckBucketExists(const std::string& bucket) const {
    Aws::S3::Model::HeadBucketRequest headBucketRequest;
    headBucketRequest.SetBucket(bucket);

    auto outcome = s3_client.HeadBucket(headBucketRequest);
    return outcome.IsSuccess();
}

bool S3Client::CreateBucket(const std::string& bucket) const {
    Aws::S3::Model::CreateBucketRequest createBucketRequest;
    createBucketRequest.SetBucket(bucket);
    auto outcome = s3_client.CreateBucket(createBucketRequest);
    if (!outcome.IsSuccess()) {
        std::cerr << outcome.GetError() << std::endl;
    }
    return outcome.IsSuccess();
}

void S3Client::UploadChunk(const std::string& bucket, const std::string& chunk_hash, const std::vector<char>& data) const {
    Aws::S3::Model::PutObjectRequest put_request;
    put_request.SetBucket(bucket.c_str());
    put_request.SetKey(chunk_hash.c_str());

    auto stream = std::make_shared<Aws::StringStream>();
    stream->write(data.data(), data.size());
    put_request.SetBody(stream);

    auto outcome = s3_client.PutObject(put_request);
    if (!outcome.IsSuccess()) {
        std::cerr << "Failed to upload chunk: " << chunk_hash << std::endl;
    }
}

uint32_t S3Client::GetChunkSize(const std::string& bucket, const std::string& chunk_hash) const {
    Aws::S3::Model::HeadObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(chunk_hash);

    auto outcome = s3_client.HeadObject(request);
    
    if (!outcome.IsSuccess()) {
        std::cerr << "Error on getting chunk size: " << outcome.GetError().GetMessage() << std::endl;
        return 0;
    }

    return outcome.GetResult().GetContentLength();
}

std::vector<char> S3Client::GetChunkData(const std::string& bucket, const std::string& chunk_hash) const {
    Aws::S3::Model::GetObjectRequest request;
    request.SetBucket(bucket);
    request.SetKey(chunk_hash);

    auto outcome = s3_client.GetObject(request);
    
    if (!outcome.IsSuccess()) {
        std::cerr << "Error getting object: " 
                  << outcome.GetError().GetMessage() << std::endl;
        return std::vector<char>();
    }

    // Чтение содержимого файла в вектор байтов
    auto& stream = outcome.GetResult().GetBody();
    stream.seekg(0, std::ios::end);
    size_t size = stream.tellg();
    stream.seekg(0, std::ios::beg);

    std::vector<char> content(size);
    stream.read(reinterpret_cast<char*>(content.data()), size);

    return content;
}
