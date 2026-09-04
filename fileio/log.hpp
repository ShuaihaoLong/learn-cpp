#pragma once

#include <chrono>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include <utility>

class Log {
public:
    enum class Level {
        Info,
        Warning,
        Error,
    };

    static Log& Instance() {
        static Log log;
        return log;
    }

    Log(const Log&) = delete;
    Log& operator=(const Log&) = delete;

    void SetLogName(std::filesystem::path name) {
        std::lock_guard<std::mutex> lock(mutex_);
        log_name_ = std::move(name);
    }

    template<typename T>
    void Info(const T& message) {
        Write(Level::Info, message);
    }

    template<typename T>
    void Warning(const T& message) {
        Write(Level::Warning, message);
    }

    template<typename T>
    void Error(const T& message) {
        Write(Level::Error, message);
    }

    template<typename T>
    void Write(Level level, const T& message) {
        std::lock_guard<std::mutex> lock(mutex_);

        WriteUnlocked(level, nullptr, 0, message);
    }

    template<typename T>
    void Write(Level level, const char* file, int line, const T& message) {
        std::lock_guard<std::mutex> lock(mutex_);

        WriteUnlocked(level, file, line, message);
    }

private:
    Log() = default;

    template<typename T>
    void WriteUnlocked(Level level, const char* file, int line, const T& message) {
        std::ofstream log(log_name_, std::ios::app);
        if(!log) {
            std::cerr << "[ERROR] Failed to open log file: " << log_name_ << '\n';
            return;
        }

        auto now = std::chrono::system_clock::now();
        std::time_t t = std::chrono::system_clock::to_time_t(now);
        std::tm tm = ToLocalTime(t);

        log << "[" << LevelName(level) << "] "
            << std::put_time(&tm, "%Y-%m-%d %H:%M:%S") << " ";

        if(file != nullptr) {
            log << file << ":" << line << " ";
        }

        log << message << '\n';
    }

    static const char* LevelName(Level level) {
        switch(level) {
            case Level::Info:
                return "INFO";
            case Level::Warning:
                return "WARNING";
            case Level::Error:
                return "ERROR";
        }

        return "UNKNOWN";
    }

    static std::tm ToLocalTime(std::time_t t) {
        std::tm tm{};

#ifdef _WIN32
        localtime_s(&tm, &t);
#else
        localtime_r(&t, &tm);
#endif

        return tm;
    }

private:
    std::filesystem::path log_name_{"test.log"};
    std::mutex mutex_;
};

#define LOG_INFO(message) (Log::Instance().Write(Log::Level::Info, __FILE__, __LINE__, message))
#define LOG_WARNING(message) (Log::Instance().Write(Log::Level::Warning, __FILE__, __LINE__, message))
#define LOG_ERROR(message) (Log::Instance().Write(Log::Level::Error, __FILE__, __LINE__, message))
