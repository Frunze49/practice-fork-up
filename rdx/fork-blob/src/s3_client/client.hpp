#include <aws/s3/S3Client.h>
#include <vector>

struct S3Config {
  std::string endpoint = getEnvOr("S3_ENDPOINT", "localhost:9000");
  std::string access_key = getEnvOr("S3_ACCESS_KEY", "cXGiKi9Otf3xtIbPGHp9");
  std::string secret_key = getEnvOr("S3_SECRET_KEY", "bDpmvYrjsaqPigTazyB9SVQGFxtk9FJeSFVp4nlb");

private:
  static std::string getEnvOr(const char* env_var, const std::string& default_value) {
      const char* value = std::getenv(env_var);
      return value ? value : default_value;
  }
};

class S3Client {
public:
  S3Client(const S3Config& s3_config);

  bool CheckChunkExists(const std::string& bucket, const std::string& chunk_hash) const;
  bool CheckBucketExists(const std::string& bucket) const;
  bool CreateBucket(const std::string& bucket) const;
  void UploadChunk(const std::string& bucket, const std::string& chunk_hash, const std::vector<char>& data) const;
  // For tests
  uint32_t GetChunkSize(const std::string& bucket, const std::string& chunk_hash) const;
  std::vector<char> GetChunkData(const std::string& bucket, const std::string& chunk_hash) const;

private:
  Aws::S3::S3Client s3_client;
};
