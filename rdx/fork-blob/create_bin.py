import os
import random
import time

def create_large_file(filename, size_in_mb):
    """Create a large binary file with random data"""
    chunk_size = 4096  # 4KB chunks for efficient writing
    total_bytes = size_in_mb * 1024 * 1024
    
    start_time = time.time()
    
    with open(filename, 'wb') as f:
        bytes_written = 0
        while bytes_written < total_bytes:
            # Generate random bytes (faster than random.getrandbits for large files)
            chunk = os.urandom(min(chunk_size, total_bytes - bytes_written))
            f.write(chunk)
            bytes_written += len(chunk)
            
            # Progress reporting
            if bytes_written % (100 * 1024 * 1024) == 0:  # Every 100MB
                mb_written = bytes_written / (1024 * 1024)
                elapsed = time.time() - start_time
                speed = mb_written / elapsed if elapsed > 0 else 0
                print(f"Progress: {mb_written:.1f}MB written ({speed:.1f}MB/s)")
    
    elapsed = time.time() - start_time
    print(f"Created {filename} ({size_in_mb}MB) in {elapsed:.2f} seconds")

if __name__ == "__main__":
    # Create a 1GB file (change size as needed)
    create_large_file("file.bin", 1)  