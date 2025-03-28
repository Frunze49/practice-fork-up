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
#include "s3_client/client.hpp"

class ForkUpTest : public ::testing::Test {
protected:
    void SetUp() override {
      Aws::InitAPI(options);

      s3_client = CreateS3Client(S3Config());
    }
    
    void TearDown() override {
      Aws::ShutdownAPI(options);
    }
    
    Aws::SDKOptions options;
    std::shared_ptr<Aws::S3::S3Client> s3_client;
};

TEST_F(ForkUpTest, CreateBucket) {
    const auto bucket_name = "test";

    const auto fork_up_provider = forkup::ForkUpProvider(s3_client);
    EXPECT_NO_THROW(fork_up_provider.ForkUp({}, bucket_name));

    Aws::S3::Model::HeadBucketRequest request;
    request.SetBucket(bucket_name);

    auto outcome = s3_client->HeadBucket(request);
    
    EXPECT_TRUE(outcome.IsSuccess());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
