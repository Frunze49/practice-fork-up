#pragma once
#include <aws/s3/S3Client.h>
#include <memory>
#include "s3_client/client.hpp"

namespace forkup {

class ForkUpProvider {
public:
    explicit ForkUpProvider(std::shared_ptr<S3Client> s3_client)
        : s3_client_(std::move(s3_client)) {}

    void ForkUp(const std::string& manifest_file_path, const std::string& binary_file_path) const;

private:
    std::shared_ptr<S3Client> s3_client_;
};

} // namespace forkup
