#pragma once

#include "types.h"
#include "log.h"
#include <cstdlib>
#include <cassert>

namespace OL {

    template<typename T>
    [[nodiscard]] static consteval size_t BITS() {
        return sizeof(T) * 8;
    }

    [[nodiscard]] static consteval size_t KB(size_t kilobytes) {
        return kilobytes * 1024;
    }

    [[nodiscard]] static consteval size_t MB(size_t megabytes) {
        return megabytes * KB(1024);
    }

    [[nodiscard]] static consteval size_t GB(size_t gigabytes) {
        return gigabytes * MB(1024);
    }

    struct Buffer {
        u8* data;
        size_t allocated;
        size_t capacity;

        [[nodiscard]] u8 operator[](size_t index) {
            return data[index];
        }

        [[nodiscard]] size_t bytes_remaining() {
            return capacity - allocated;
        }

        template<typename T = u8>
        [[nodiscard]] T* reserve(size_t element_count) {
            assert(element_count > 0);
            if(element_count == 0) {
                Logger::log(Logger::LEVEL_ERROR, "[Buffer::reserve()] Attempt to allocate 0 elements.");
                return nullptr;
            }

            size_t elements_size = sizeof(T) * element_count;
            size_t remaining_capacity = capacity - allocated;
            assert(elements_size <= remaining_capacity);
            if(elements_size > remaining_capacity) {
                Logger::log(Logger::LEVEL_ERROR, "[Buffer::reserve()] Not enough capacity to allocate %zu elements.", element_count);
                return nullptr;
            }

            T* pointer = reinterpret_cast<T*>(data + allocated);
            allocated += elements_size;

            return pointer;
        }

        template<typename T = u8>
        [[nodiscard]] T* reserve() {
            return reserve<T>(1);
        }

        [[nodiscard]] static Buffer* allocate(size_t bytes) {
            assert(bytes > 0);
            if(bytes == 0) {
                Logger::log(Logger::LEVEL_ERROR, "[Buffer::allocate()] Attempt to allocate 0 bytes.");
                return nullptr;
            }

            Buffer* buffer = reinterpret_cast<Buffer*>(malloc(sizeof(Buffer)));
            if(!buffer) {
                Logger::log(Logger::LEVEL_ERROR, "[Buffer::allocate()] Failed to allocate memory for Buffer struct.");
                return nullptr;
            }

            void* data = malloc(bytes);
            if(!data) {
                Logger::log(Logger::LEVEL_ERROR, "[Buffer::allocate()] Failed to allocate Buffer with %zu bytes.", bytes);
                free(buffer);
                return nullptr;
            }

            buffer->data = reinterpret_cast<u8*>(data);
            buffer->capacity = bytes;
            buffer->allocated = 0;

            return buffer;
        }

        [[nodiscard]] static Buffer* slice(Buffer* parent, size_t capacity) {
            assert(parent);
            assert(capacity > 0);

            Buffer* slice = parent->reserve<Buffer>();
            assert(slice);
            if(!slice) {
                return nullptr;
            }

            slice->data = parent->reserve(capacity);
            assert(slice->data);
            if(!slice->data) {
                return nullptr;
            }

            slice->capacity = capacity;
            slice->allocated = 0;
            return slice;
        }
    };

    struct Memory {
        static constexpr size_t DEFAULT_PERSISTENT_MEMORY_SIZE = MB(10);
        static constexpr size_t DEFAULT_TEMPORARY_MEMORY_SIZE = MB(10);

        Buffer* persistent;
        Buffer* temporary;
    };

    static Memory memory = {
        .persistent = Buffer::allocate(Memory::DEFAULT_PERSISTENT_MEMORY_SIZE),
        .temporary = Buffer::allocate(Memory::DEFAULT_TEMPORARY_MEMORY_SIZE)
    };
}