#include "fork_up.hpp"
#include <aws/s3/model/CreateBucketRequest.h>

namespace forkup {

void ForkUpProvider::ForkUp(const Manifest& manifest, const std::string& binary_file_path) const {
  Aws::S3::Model::CreateBucketRequest request;
  request.SetBucket(binary_file_path);

  
  auto outcome = s3_client_->CreateBucket(request);
  if (!outcome.IsSuccess()) {
      throw std::runtime_error("Failed to create bucket with error: " + outcome.GetError().GetMessage());
  }
}

} // namespace forkup