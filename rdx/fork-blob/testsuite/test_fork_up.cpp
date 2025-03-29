#include <gtest/gtest.h>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/core/utils/UUID.h>

#include <cstdlib>
#include <filesystem>

#include "fork_up.hpp"
#include "file_helper/helper.hpp"

class ForkUpTest : public ::testing::Test {
protected:
    void SetUp() override {
      Aws::InitAPI(options);
      s3_client = std::make_shared<S3Client>(S3Config());
    }
    
    void TearDown() override {
      Aws::ShutdownAPI(options);
    }
    
    Aws::SDKOptions options;
    std::shared_ptr<S3Client> s3_client;
};

TEST_F(ForkUpTest, CorrectReading) {
  const auto binary_file_path = "testsuite/new/small.bin";
  const auto manifest_file_path = "testsuite/new/small.manifest";

  Manifest manifest(manifest_file_path);
  Blob binary(binary_file_path);

  // check correct manifest reading
  //       +
  //       correct blob_data size reading
  auto blob = manifest.GetNextBlock();
  auto blob_data = binary.ReadNextData(blob->chunk_size);

  ASSERT_EQ(blob->chunk_size, 67102);
  ASSERT_EQ(blob->chunk_hash, "187496e2fe5fb510b577f71525f9d0c05521ef0381fa1de3a8aefefaa9011125");
  ASSERT_EQ(blob_data.size(), blob->chunk_size);


  blob = manifest.GetNextBlock();
  blob_data = binary.ReadNextData(blob->chunk_size);

  ASSERT_EQ(blob->chunk_size, 55887);
  ASSERT_EQ(blob->chunk_hash, "5473684f1a2b1f5a1603b90aadc36e2bcd8860114f17eff5519727baf0846ec1");
  ASSERT_EQ(blob_data.size(), blob->chunk_size);

  // ...
  // check end

  while(auto cycle_blob = manifest.GetNextBlock()) {
    blob = cycle_blob;
    blob_data = binary.ReadNextData(cycle_blob->chunk_size);
  }

  ASSERT_EQ(blob->chunk_size, 60656);
  ASSERT_EQ(blob->chunk_hash, "111f1d19b475d493c49860ac8b707914bb1e9d7eb79e3694b504e3a97c741fc0");
  ASSERT_EQ(blob_data.size(), blob->chunk_size);
}

TEST_F(ForkUpTest, ForkUpSmallNewFile) {
    const auto binary_file_path = "testsuite/new/small.bin";
    const auto manifest_file_path = "testsuite/new/small.manifest";

    const auto fork_up_provider = forkup::ForkUpProvider(s3_client);
    EXPECT_NO_THROW(fork_up_provider.ForkUp(manifest_file_path, binary_file_path));

    Manifest manifest(manifest_file_path);
    Blob binary(binary_file_path);
    std::string bucket = std::filesystem::path(binary_file_path).filename().string();

    while (const auto blob = manifest.GetNextBlock()) {
      ASSERT_TRUE(s3_client->CheckChunkExists(bucket, blob->chunk_hash));
      ASSERT_EQ(s3_client->GetChunkSize(bucket, blob->chunk_hash), blob->chunk_size);
      ASSERT_EQ(s3_client->GetChunkData(bucket, blob->chunk_hash), binary.ReadNextData(blob->chunk_size));
    }
}

// TEST_F(ForkUpTest, ForkUpSmallModifiedFile) {
//   // replace some data
//   const auto binary_file_path = "testsuite/modified/small.bin";
//   const auto manifest_file_path = "testsuite/modified/small.manifest";

//   const auto fork_up_provider = forkup::ForkUpProvider(s3_client);
//   EXPECT_NO_THROW(fork_up_provider.ForkUp(manifest_file_path, binary_file_path));

//   Manifest manifest(manifest_file_path);
//   Blob binary(binary_file_path);
//   std::string bucket = std::filesystem::path(binary_file_path).filename().string();

//   while (const auto blob = manifest.GetNextBlock()) {
//     ASSERT_TRUE(s3_client->CheckChunkExists(bucket, blob->chunk_hash));
//     ASSERT_EQ(s3_client->GetChunkSize(bucket, blob->chunk_hash), blob->chunk_size);
//     ASSERT_EQ(s3_client->GetChunkData(bucket, blob->chunk_hash), binary.ReadNextData(blob->chunk_size));
//   }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}