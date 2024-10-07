#pragma once

#include <cstdio>

namespace OL {
    namespace Logger {

        static constexpr size_t MAX_LOG_MESSAGE_LENGTH = 256;

        using LogLevel = size_t;
        static constexpr LogLevel LEVEL_DEBUG = 0;
        static constexpr LogLevel LEVEL_ERROR = 1;
        static constexpr LogLevel LEVEL_SILENT = 2;

        static constexpr LogLevel DEFAULT_LOGGER_LEVEL = LEVEL_DEBUG;
        static LogLevel logger_level = DEFAULT_LOGGER_LEVEL;

        [[nodiscard]] static const char* log_level_to_string(LogLevel level) {
            switch(level) {
                case LEVEL_DEBUG: {
                    return "[Debug]";
                } break;
                case LEVEL_ERROR: {
                    return "[Error]";
                } break;
                default: {
                    return "";
                }
            }
        };

        template<typename... Args>
        static void log(LogLevel message_level, const char* format, Args... args) {
            if(message_level < logger_level) {
                return;
            }

            char buffer[MAX_LOG_MESSAGE_LENGTH];
            snprintf(buffer, MAX_LOG_MESSAGE_LENGTH, "%s%s\n", log_level_to_string(message_level), format);
            printf(buffer, args...);
        }

        template<typename... Args>
        static void log(const char* format, Args... args) {
            log(logger_level, format, args...);
        }
    }
}