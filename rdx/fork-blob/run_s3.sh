docker run -d \
  -p 9002:9000 -p 9001:9001 \
  --name minio \
  -v /mnt/data:/data \
  -e "MINIO_ROOT_USER=admin" \
  -e "MINIO_ROOT_PASSWORD=admin" \
  quay.io/minio/minio server --console-address ":9001" /data

# mc alias set local http://localhost:9002 admin admin
# mc mb local/my-bucket
# mc ls local

# cmake -B build -DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake
# cmake --build build