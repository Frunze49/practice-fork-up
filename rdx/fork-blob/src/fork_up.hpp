#pragma once
#include <unordered_map>
#include <aws/s3/S3Client.h>

#include "s3_client/client.hpp"
#include "file_helper/helper.hpp"

namespace forkup {

class ForkUpProvider {
public:
    explicit ForkUpProvider(S3Client&& s3_client)
        : s3_client_(std::move(s3_client)) {}

    void ForkUp(const std::string& manifest_file_path, const std::string& binary_file_path) const;

private:
    S3Client s3_client_;
};

} // namespace forkup