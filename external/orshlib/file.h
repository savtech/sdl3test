#pragma once

#include "memory.h"
#include "util.h"
#include <cstdio>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace OL {

    struct File {
#ifdef _WIN32
        static constexpr size_t MAX_FILENAME_LENGTH = MAX_PATH;
#else
        static constexpr size_t MAX_FILENAME_LENGTH = 256;
#endif

        char name[MAX_FILENAME_LENGTH];
        Buffer* buffer;

        u8* data() {
            return buffer->data;
        };

        size_t size() {
            return buffer->allocated;
        };

        [[nodiscard]] static File* load(const char* path, Buffer* buffer = memory.temporary) {
            if(!buffer) {
                Logger::log(Logger::LEVEL_ERROR, "[File::load()] Invalid buffer provided, no space to load file.");
                return nullptr;
            }

            File* handle = buffer->reserve<File>();
            FILE* stream;

            s32 result = fopen_s(&stream, path, "rb");
            if(result != 0) {
                char error_string[128];
                strerror_s(error_string, 128, result);
                Logger::log(Logger::LEVEL_ERROR, "[File::load()] Unable to open %s...%s\n", path, error_string);
                return nullptr;
            }

            memcpy_s(handle->name, MAX_FILENAME_LENGTH, path, string_length(path) + 1);

            fseek(stream, 0, SEEK_END);
            size_t file_size = ftell(stream);
            rewind(stream);

            handle->buffer = Buffer::slice(buffer, file_size);
            if(!handle->buffer->data) {
                return nullptr;
            }

            size_t bytes_read = fread_s(handle->buffer->data, handle->buffer->capacity, sizeof(u8), handle->buffer->capacity, stream);
            if(bytes_read != handle->buffer->capacity) {
                Logger::log(Logger::LEVEL_ERROR, "[File::load()] Failed to read all file data.");
                return nullptr;
            }

            handle->buffer->allocated = bytes_read;

            fclose(stream);

            return handle;
        }

        static size_t write_to_file(const char* path, Buffer* buffer, size_t offset, size_t bytes) {
            size_t bytes_written = 0;

            FILE* stream;
            s32 result = fopen_s(&stream, path, "wb");
            if(result != 0) {
                char error_string[128];
                strerror_s(error_string, 128, result);
                Logger::log(Logger::LEVEL_ERROR, "[File::load()] Unable to open %s...%s\n", path, error_string);
                return 0;
            }

            bytes_written = fwrite(buffer->data + offset, sizeof(u8), bytes, stream);
            fclose(stream);

            return bytes_written;
        }
    };
}