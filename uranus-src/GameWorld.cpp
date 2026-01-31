#include "GameWorld.h"

#include <config/ConfigModule.h>

#include <ranges>
#include <format>
#include <asio/signal_set.hpp>
#include <yaml-cpp/yaml.h>
#include <spdlog/spdlog.h>


using uranus::config::ConfigModule;

namespace uranus {
    GameWorld::GameWorld()
        : guard_(asio::make_work_guard(ctx_)) {
    }

    GameWorld::~GameWorld() {

    }

    void GameWorld::run() {
        for (const auto &val : ordered_) {
            SPDLOG_INFO("Start module: {}", val->getModuleName());
            val->start();
        }

        int num = 0;

        {
            const auto *config = GET_MODULE(this, ConfigModule);
            if (!config) {
                SPDLOG_ERROR("Config module not found!");
                exit(-1);
            }

            // Get global server config
            const auto &cfg = config->getServerConfig();

            // Read the worker threads number
            num = cfg["server"]["worker"]["threads"].as<int>();
        }

        pool_.start(num);
        SPDLOG_INFO("Worker pool start with {} thread(s)", num);

        asio::signal_set signals(ctx_, SIGINT, SIGTERM);
        signals.async_wait([this](auto, auto) {
            this->terminate();
        });

        SPDLOG_INFO("GameWorld is running...");
        ctx_.run();
    }

    void GameWorld::terminate() {
        if (ctx_.stopped())
            return;

        // Shutdown the workers pool
        pool_.stop();

        // Shutdown all modules
        for (const auto val : ordered_ | std::views::reverse) {
            SPDLOG_INFO("Stop module: {}", val->getModuleName());
            val->stop();
        }

        ordered_.clear();
        modules_.clear();

        // Shutdown the main io_context
        guard_.reset();
        ctx_.stop();

        SPDLOG_INFO("GameWorld terminated");
    }

    bool GameWorld::isRunning() const {
        return !ctx_.stopped();
    }

    asio::io_context &GameWorld::getIOContext() {
        return ctx_;
    }

    asio::io_context &GameWorld::getWorkerIOContext() {
        return pool_.getIOContext();
    }

    // void GameWorld::pushModule(ServerModule *module) {
    //     if (!module)
    //         return;
    //
    //     const auto name = module->getModuleName();
    //     if (modules_.contains(name)) {
    //         throw std::logic_error(std::format("ServerModule[{}] already exists!", name));
    //     }
    //
    //     modules_.insert_or_assign(name, unique_ptr<ServerModule>(module));
    //     ordered_.emplace_back(module);
    // }

    ServerModule *GameWorld::getModule(const std::string &name) const {
        const auto iter = modules_.find(name);
        return iter != modules_.end() ? iter->second.get() : nullptr;
    }
}
