#pragma once

namespace OL {
    template<typename T, size_t N>
    [[nodiscard]] static constexpr size_t array_count(T (&)[N]) {
        return N;
    }

    [[nodiscard]] static constexpr size_t string_length(const char* string) {
        if(!string) {
            return 0;
        }

        size_t count = 0;
        while(*string++) {
            ++count;
        }

        return count;
    }
}