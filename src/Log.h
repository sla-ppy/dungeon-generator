#pragma once

#include <chrono>
#include <fmt/chrono.h>
#include <fmt/color.h>
#include <fmt/core.h>

// quick & dirty logging

namespace l {

template<typename... Args>
inline void info(Args&&... args) {
    fmt::print("{}    {} {} {}\n",
        fmt::styled(std::chrono::high_resolution_clock::now(), fmt::fg(fmt::color::gray)),
        fmt::styled("INFO", fmt::fg(fmt::color::green)),
        fmt::styled("|", fmt::fg(fmt::color::gray)),
        fmt::format(std::forward<Args>(args)...));
}

template<typename... Args>
inline void warning(Args&&... args) {
    fmt::print("{} {} {} {}\n",
        fmt::styled(std::chrono::high_resolution_clock::now(), fmt::fg(fmt::color::gray)),
        fmt::styled("WARNING", fmt::fg(fmt::color::yellow)),
        fmt::styled("|", fmt::fg(fmt::color::gray)),
        fmt::format(std::forward<Args>(args)...));
}

template<typename... Args>
inline void error(Args&&... args) {
    fmt::print("{}   {} {} {}\n",
        fmt::styled(std::chrono::high_resolution_clock::now(), fmt::fg(fmt::color::gray)),
        fmt::styled("ERROR", fmt::fg(fmt::color::red)),
        fmt::styled("|", fmt::fg(fmt::color::gray)),
        fmt::format(std::forward<Args>(args)...));
}

}
