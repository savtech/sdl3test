#pragma once

#include "types.h"
#include "time.h"

namespace OL {

    struct FPS {
        static constexpr Time::Duration MEASUREMENT_INTERVAL = Time::Milliseconds(300);

        Time::Stamp measurement_start_time = Time::Clock::now();
        u32 frames = 0;
        f64 last_measurement = 0.0;
        Timer timer = { .interval = MEASUREMENT_INTERVAL };
    };

    struct Session {
        u64 frames = 0;
        u64 ticks = 0;
        Time::Stamp start = Time::Clock::now();
        FPS fps = {};
        bool display_fps = false;

        [[nodiscard]] Time::Duration get_running_time() {
            return Time::Clock::now() - start;
        }

        void update([[maybe_unused]] Time::Duration delta_time) {
            ++ticks;
        }

        void render(Time::Duration delta_time) {
            ++frames;
            ++fps.frames;

            fps.timer.accumulate(delta_time);
            if(fps.timer.ready()) {
                Time::Stamp now = Time::Clock::now();
                f64 measurement_delta = std::chrono::duration_cast<std::chrono::duration<f64, std::chrono::seconds::period>>(now - fps.measurement_start_time).count();
                fps.last_measurement = static_cast<f64>((fps.frames / measurement_delta));
                fps.measurement_start_time = now;
                fps.frames = 0;
                fps.timer.consume();
            }
        }

        void debug_print() {
            Logger::log("Session Info:");

            f64 elapsed_time_seconds = static_cast<f64>(get_running_time().count() / 1'000'000'000.0); //1 BILLION nanoseconds in a second
            u32 hours = static_cast<u32>(elapsed_time_seconds / 3600);
            u32 minutes = static_cast<u32>((elapsed_time_seconds - (hours * 3600)) / 60);
            f64 seconds = elapsed_time_seconds - (hours * 3600) - (minutes * 60);

            Logger::log("Elapsed Time: %02d:%02d:%05.2f", hours, minutes, seconds);
            Logger::log("Total Frames: %zd", frames);
            Logger::log("Average FPS: %00007.2f", static_cast<f64>(frames / elapsed_time_seconds));
        }
    };
}