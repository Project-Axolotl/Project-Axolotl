// Copyright (c) 2022 Dániel Gergely, Dénes Balogh
// Distributed under the MIT License.

#pragma once
#include "common.hpp"

namespace cli::print {
    enum class logLevels {
        fatal,
        error,
        warning,
        notice,
        info,
        debug,
        trace
    };

    // default level
    logLevels GLOBAL_LOG_LEVEL = cli::print::logLevels::info;

    enum colors {
        fg_black    = 30,
        fg_red      = 31,
        fg_green    = 32,
        fg_yellow   = 33,
        fg_blue     = 34,
        fg_magenta  = 35,
        fg_cyan     = 36,
        fg_white    = 37
    };

    enum modifiers {
        reset       = 0,
        bold        = 1,
        underline   = 4,
        inverse     = 7
    };

    void setLogLevel (logLevels level) {
        GLOBAL_LOG_LEVEL = level;
    }

    template <typename T> void buffer (colors color, T data, modifiers modifier = reset) {
        std::cout << "\033[" << modifier << ";" << color << "m" << data << "\033[0m";
    }

    void flush () {
        std::cout << "\033[0m\n" << std::flush;
    }

    std::string time () {
        auto now = std::chrono::system_clock::now();
        auto in_time_t = std::chrono::system_clock::to_time_t(now);
        std::stringstream ss;
        ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
        return ss.str();
    }

    template <typename T> void trace (T data) {
        if (GLOBAL_LOG_LEVEL < logLevels::trace) return;
        buffer(fg_white, "[TRACE] " + time() + " - " + data);
        flush();
    }

    template <typename T> void debug (T data) {
        if (GLOBAL_LOG_LEVEL < logLevels::debug) return;
        buffer(fg_white, "[DEBUG] " + time() + " - " + data);
        flush();
    }

    template <typename T> void info (T data) {
        if (GLOBAL_LOG_LEVEL < logLevels::info) return;
        buffer(fg_white, "[INFO] " + time() + " - " + data);
        flush();
    }

    template <typename T> void notice (T data) {
        if (GLOBAL_LOG_LEVEL < logLevels::notice) return;
        buffer(fg_cyan, "[NOTICE] " + time() + " - ");
        buffer(fg_white, data);
        flush();
    }

    template <typename T> void warning (T data) {
        if (GLOBAL_LOG_LEVEL < logLevels::warning) return;
        buffer(fg_yellow, "[WARNING] " + time() + " - ");
        buffer(fg_white, data);
        flush();
    }

    template <typename T> void error (T data) {
        if (GLOBAL_LOG_LEVEL < logLevels::error) return;
        buffer(fg_red, "[ERROR] " + time() + " - ");
        buffer(fg_white, data);
        flush();
    }

    template <typename T> void fatal (T data) {
        buffer(fg_red, "[FATAL] " + time() + " - ", bold);
        buffer(fg_red, data, bold);
        flush();
        exit(1);
    }
}