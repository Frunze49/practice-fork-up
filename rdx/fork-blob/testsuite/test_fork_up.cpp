#include <gtest/gtest.h>

#include <aws/core/Aws.h>
#include <aws/core/auth/AWSCredentials.h>
#include <aws/s3/S3Client.h>
#include <aws/s3/model/CreateBucketRequest.h>
#include <aws/s3/model/HeadBucketRequest.h>
#include <aws/s3/model/DeleteBucketRequest.h>
#include <aws/core/utils/UUID.h>

#include <cstdlib>

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

TEST_F(ForkUpTest, CreateBucket) {
    const auto binary_file_path = "./testsuite/results/small.bin";
    const auto manifest_file_path = "./testsuite/results/small.manifest";

    const auto fork_up_provider = forkup::ForkUpProvider(s3_client);
    EXPECT_NO_THROW(fork_up_provider.ForkUp(binary_file_path, binary_file_path));

    Manifest manifest(manifest_file_path);
    while (const auto blob = manifest.GetNextBlock()) {
      ASSERT_TRUE(s3_client->CheckChunkExists(binary_file_path, blob->chunk_hash));
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}