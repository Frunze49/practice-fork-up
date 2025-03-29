#include <aws/s3/model/CreateBucketRequest.h>

#include "fork_up.hpp"
#include "file_helper/helper.hpp"

#include <filesystem>

namespace forkup {

void ForkUpProvider::ForkUp(const std::string& manifest_file_path, const std::string& binary_file_path) const {
  Manifest manifest(manifest_file_path);
  Blob blob(binary_file_path);

  std::string bucket = std::filesystem::path(binary_file_path).filename().string();

  if (!s3_client_->CheckBucketExists(bucket)) {
    s3_client_->CreateBucket(bucket);
  }

  auto current_chunk_info = manifest.GetNextBlock();
  while (current_chunk_info.has_value()) {
      auto ch_size = current_chunk_info.value().chunk_size;
      auto ch_hash = current_chunk_info.value().chunk_hash;
      auto data_to_write = blob.ReadNextData(current_chunk_info.value().chunk_size);

      if (!s3_client_->CheckChunkExists(bucket, ch_hash)) {
          s3_client_->UploadChunk(bucket, ch_hash, data_to_write);
      }

      current_chunk_info = manifest.GetNextBlock();
  }
}

} // namespace forkup