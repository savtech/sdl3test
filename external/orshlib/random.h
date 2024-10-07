#pragma once

#include "types.h"
#include <random>

namespace OL {

    namespace Random {

        [[nodiscard]] static std::mt19937& seed() {
            thread_local static std::random_device randomDevice;
            thread_local static std::mt19937 random(randomDevice());

            return random;
        }

        [[nodiscard]] [[maybe_unused]] static u32 U32(u32 min, u32 max) {
            auto distribution = std::uniform_int_distribution<u32>(min, max);
            return distribution(seed());
        }

        [[nodiscard]] [[maybe_unused]] static f32 F32(f32 min, f32 max) {
            auto distribution = std::uniform_real_distribution<f32>(min, max);
            return distribution(seed());
        }

    }
}