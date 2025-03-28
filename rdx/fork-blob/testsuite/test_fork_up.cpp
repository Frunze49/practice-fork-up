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

class ForkUpTest : public ::testing::Test {
protected:
    ForkUpTest() : s3_client(S3Config()) {}

    void SetUp() override {
      Aws::InitAPI(options);
    }
    
    void TearDown() override {
      Aws::ShutdownAPI(options);
    }
    
    Aws::SDKOptions options;
    S3Client s3_client;
};

TEST_F(ForkUpTest, CreateBucket) {
    const auto bucket_name = "test";

    const auto fork_up_provider = forkup::ForkUpProvider(s3_client);
    EXPECT_NO_THROW(fork_up_provider.ForkUp({}, bucket_name));

    // auto outcome = s3_client->HeadBucket(request);
    
    // EXPECT_TRUE(outcome.IsSuccess());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
