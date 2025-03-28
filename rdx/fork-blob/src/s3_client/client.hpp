#include <aws/s3/S3Client.h>

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


std::shared_ptr<Aws::S3::S3Client> CreateS3Client(const S3Config& s3_config);
