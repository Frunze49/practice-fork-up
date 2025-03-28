#include "fork_up.hpp"
#include <aws/s3/model/CreateBucketRequest.h>

namespace forkup {

void ForkUpProvider::ForkUp(const std::string& manifest_file_path, const std::string& binary_file_path) const {
  Manifest manifest(manifest_file_path);
  Blob blob(binary_file_path);

  auto current_chunk_info = manifest.GetNextBlock();
  while (current_chunk_info.has_value()) {
      auto ch_size = current_chunk_info.value().chunk_size;
      auto ch_hash = current_chunk_info.value().chunk_hash;

      auto data_to_write = blob.ReadNextData(current_chunk_info.value().chunk_size);

      if (!s3_client_.CheckChunkExists(binary_file_path, ch_hash)) {
          s3_client_.UploadChunk(binary_file_path, ch_hash, data_to_write);
      }

      current_chunk_info = manifest.GetNextBlock();
  }
}

} // namespace forkup