#pragma once
#include <unordered_map>
#include <aws/s3/S3Client.h>

namespace forkup {

using Manifest = std::unordered_map<std::string, std::string>;

class ForkUpProvider {
public:
    explicit ForkUpProvider(std::shared_ptr<Aws::S3::S3Client> s3_client)
        : s3_client_(std::move(s3_client)) {}

    void ForkUp(const Manifest& manifest, const std::string& binary_file_path) const;

private:
    const std::shared_ptr<Aws::S3::S3Client> s3_client_;
};

} // namespace forkup