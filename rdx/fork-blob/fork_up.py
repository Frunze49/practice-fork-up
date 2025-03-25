import sys
import os
import json
import hashlib
import concurrent.futures
from typing import List, Dict, Tuple
from minio import Minio
from minio.error import S3Error
import io

def parse_manifest(manifest_path: str) -> List[Tuple[int, str]]:
    """Parse the RDX manifest file"""
    with open(manifest_path, 'r') as f:
        manifest_data = json.load(f)
    
    chunks = []
    for item in manifest_data:
        size, hash_val = item.split(':', 1)
        chunks.append((int(size), hash_val))
    
    return chunks

def extract_chunks_from_file(file_path: str, chunks: List[Tuple[int, str]]) -> Dict[str, bytes]:
    """Extract chunks from the file based on the manifest"""
    chunk_data = {}
    offset = 0
    
    with open(file_path, 'rb') as f:
        for size, hash_val in chunks:
            f.seek(offset)
            data = f.read(size)
            
            # Verify the hash
            calculated_hash = hashlib.sha256(data).hexdigest()
            if calculated_hash != hash_val:
                raise ValueError(f"Hash mismatch for chunk at offset {offset}. Expected {hash_val}, got {calculated_hash}")
            
            chunk_data[hash_val] = data
            offset += size
    
    return chunk_data

def upload_chunk(minio_client, bucket: str, chunk_hash: str, data: bytes) -> bool:
    """Upload a single chunk to MinIO"""
    try:
        # Check if chunk already exists
        try:
            minio_client.stat_object(bucket, chunk_hash)
            print(f"Chunk {chunk_hash[:8]}... already exists, skipping upload")
            return True
        except S3Error as err:
            if err.code != 'NoSuchKey':
                raise
        
        # Upload the chunk
        data_stream = io.BytesIO(data)
        minio_client.put_object(
            bucket_name=bucket,
            object_name=chunk_hash,
            data=data_stream,
            length=len(data)
        )
        print(f"Uploaded chunk {chunk_hash[:8]}... ({len(data)} bytes)")
        return True
    except Exception as e:
        print(f"Error uploading chunk {chunk_hash[:8]}...: {e}")
        return False

def ensure_bucket_exists(minio_client, bucket: str):
    """Make sure the bucket exists, create it if it doesn't"""
    try:
        if not minio_client.bucket_exists(bucket):
            minio_client.make_bucket(bucket)
            print(f"Created bucket: {bucket}")
        else:
            print(f"Bucket {bucket} already exists")
    except Exception as e:
        print(f"Error checking/creating bucket: {e}")
        raise

def main():
    if len(sys.argv) != 4:
        print(f"Usage: {sys.argv[0]} <manifest_path> <file_path> <minio_uri>")
        sys.exit(1)
    
    manifest_path = sys.argv[1]
    file_path = sys.argv[2]
    minio_uri = sys.argv[3]
    
    # Extract endpoint, bucket, and optional prefix from MinIO URI
    # Format: minio://<endpoint>/<bucket>[/prefix]
    if not minio_uri.startswith("minio://"):
        print("Error: MinIO URI must start with minio://")
        sys.exit(1)
    
    uri_parts = minio_uri[8:].split('/', 2)
    if len(uri_parts) < 2:
        print("Error: MinIO URI must include endpoint and bucket")
        sys.exit(1)
        
    endpoint = uri_parts[0]
    bucket = uri_parts[1]
    prefix = uri_parts[2] if len(uri_parts) > 2 else ""
    
    # Parse the manifest
    try:
        chunks = parse_manifest(manifest_path)
        print(f"Manifest contains {len(chunks)} chunks")
    except Exception as e:
        print(f"Error parsing manifest: {e}")
        sys.exit(1)
    
    # Extract chunks from the file
    try:
        chunk_data = extract_chunks_from_file(file_path, chunks)
        print(f"Successfully extracted {len(chunk_data)} chunks from file")
    except Exception as e:
      print(f"Error extracting chunks: {e}")
        sys.exit(1)
    
    # Initialize MinIO client
    # For MinIO, typically endpoint would be like localhost:9000
    try:
        # Determine if using TLS (secure) based on endpoint
        secure = not endpoint.startswith("localhost") and not endpoint.startswith("127.0.0.1")
        
        # Get credentials from environment variables
        access_key = os.environ.get('MINIO_ACCESS_KEY', 'minioadmin')
        secret_key = os.environ.get('MINIO_SECRET_KEY', 'minioadmin')
        
        minio_client = Minio(
            endpoint=endpoint,
            access_key=access_key,
            secret_key=secret_key,
            secure=secure
        )
        
        # Ensure the bucket exists
        ensure_bucket_exists(minio_client, bucket)
        
    except Exception as e:
        print(f"Error initializing MinIO client: {e}")
        sys.exit(1)
    
    # Upload chunks in parallel
    success_count = 0
    failure_count = 0
    
    with concurrent.futures.ThreadPoolExecutor(max_workers=10) as executor:
        futures = {
            executor.submit(upload_chunk, minio_client, bucket, chunk_hash, data): chunk_hash
            for chunk_hash, data in chunk_data.items()
        }
        
        for future in concurrent.futures.as_completed(futures):
            chunk_hash = futures[future]
            try:
                success = future.result()
                if success:
                    success_count += 1
                else:
                    failure_count += 1
            except Exception as e:
                print(f"Exception uploading chunk {chunk_hash[:8]}...: {e}")
                failure_count += 1
    
    # Upload the manifest if a prefix was provided
    if prefix:
        try:
            manifest_key = f"{prefix.rstrip('/')}/manifest.json"
            with open(manifest_path, 'r') as f:
                manifest_content = f.read()
            
            manifest_data = io.BytesIO(manifest_content.encode())
            minio_client.put_object(
                bucket_name=bucket,
                object_name=manifest_key,
                data=manifest_data,
                length=len(manifest_content)
            )
            print(f"Uploaded manifest to {bucket}/{manifest_key}")
            
            # Calculate manifest hash and store it as a reference
            manifest_hash = hashlib.sha256(manifest_content.encode()).hexdigest()
            hash_key = f"{prefix.rstrip('/')}/{manifest_hash}"
            
            manifest_data.seek(0)
            minio_client.put_object(
                bucket_name=bucket,
                object_name=hash_key,
                data=manifest_data,
                length=len(manifest_content)
            )
            print(f"Uploaded manifest hash reference to {bucket}/{hash_key}")
            
        except Exception as e:
            print(f"Error uploading manifest: {e}")
    
    print(f"\nSummary:")
    print(f"  Total chunks: {len(chunk_data)}")
    print(f"  Successfully uploaded: {success_count}")
    print(f"  Failed to upload: {failure_count}")
    
    if failure_count > 0:
        sys.exit(1)

if name == "main":
    main()
