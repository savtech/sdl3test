#pragma once

#include "types.h"
#include <chrono>

namespace OL {

    namespace Time {

        using Nanoseconds = std::chrono::duration<s64, std::nano>;
        using Milliseconds = std::chrono::duration<s64, std::milli>;
        using Seconds = std::chrono::duration<s64>;

        using Clock = std::chrono::steady_clock;
        using Stamp = Clock::time_point;
        using Duration = Nanoseconds;
    }

    struct Timer {
        static constexpr Time::Duration DEFAULT_INTERVAL = Time::Seconds(1);

        Time::Duration interval = DEFAULT_INTERVAL;
        Time::Duration accumulator = Time::Duration::zero();
        u64 cycles = 0;

        [[nodiscard]] bool ready() {
            return accumulator >= interval;
        }

        [[nodiscard]] Time::Duration remaining() {
            return interval - accumulator;
        }

        void accumulate(Time::Duration delta) {
            accumulator += delta;
        };

        void reset() {
            accumulator = Time::Duration::zero();
        }

        void consume() {
            if(ready()) {
                accumulator -= interval;
                ++cycles;
            }
        }
    };

}