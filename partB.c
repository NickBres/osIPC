#include "partB.h"

void generate_file(char* filename, long size_in_bytes) {
    FILE* fp = fopen(filename, "wb");
    if (!fp) {
        printf("Error opening file '%s'\n", filename);
        return;
    }

    const int chunk_size = 1024 * 1024; // 1MB
    char buffer[chunk_size];
    int bytes_written = 0;

    while (bytes_written < size_in_bytes) {
        int bytes_to_write = chunk_size;
        if (bytes_written + bytes_to_write > size_in_bytes) {
            bytes_to_write = size_in_bytes - bytes_written;
        }
        fwrite(buffer, bytes_to_write, 1, fp);
        bytes_written += bytes_to_write;
    }

    fclose(fp);
    printf("Generated file '%s' of size %ld bytes\n", filename, size_in_bytes);
}

uint32_t generate_checksum(char* filename) {
    FILE* fp = fopen(filename, "rb");
    if (!fp) {
        printf("Error opening file '%s'\n", filename);
        return 0;
    }

    const int chunk_size = 1024 * 1024; // 1MB
    char buffer[chunk_size];
    uint32_t checksum = 0;

    while (!feof(fp)) {
        size_t bytes_read = fread(buffer, 1, chunk_size, fp);
        for (size_t i = 0; i < bytes_read; i++) {
            checksum += (uint32_t)buffer[i];
        }
    }

    fclose(fp);
    printf("Generated checksum for file '%s': 0x%08x\n", filename, checksum);
    return checksum;
}

int delete_file(char* filename) {
    int status = remove(filename);
    if (status != 0) {
        printf("Error deleting file '%s'\n", filename);
        return -1;
    }
    printf("File '%s' deleted successfully\n", filename);
    return 0;
}
