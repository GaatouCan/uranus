#include "logger/LoggerModule.h"

#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/daily_file_sink.h>

namespace uranus::logger {
    LoggerModule::LoggerModule() {
        console_ = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        console_->set_level(spdlog::level::info);
    }

    LoggerModule::~LoggerModule() {
        SPDLOG_DEBUG("Destroy LoggerModule");
    }

    void LoggerModule::start() {
    }

    void LoggerModule::stop() {
    }

    shared_ptr<spdlog::logger> LoggerModule::createLogger(const std::string_view name, const std::string_view path) {
        if (auto logger = spdlog::get(name.data()); logger != nullptr) {
            return logger;
        }

        auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(std::string("logs/") + path.data() + ".txt", 2, 3);
        fileSink->set_level(spdlog::level::info);

        auto logger = std::make_shared<spdlog::logger>(name.data(), spdlog::sinks_init_list{console_, fileSink});
        logger->set_level(spdlog::level::debug);

        spdlog::register_logger(logger);
        return logger;
    }
}
