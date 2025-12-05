#include "GameWorld.h"

#include <ranges>
#include <asio/signal_set.hpp>
#include <spdlog/spdlog.h>


GameWorld::GameWorld()
    : guard_(asio::make_work_guard(ctx_)) {
}

GameWorld::~GameWorld() {
    terminate();
}

void GameWorld::run() {
    for (const auto &val: ordered_) {
        SPDLOG_INFO("Starting module: {}", val->getModuleName());
        val->start();
    }

    pool_.start();

    asio::signal_set signals(ctx_, SIGINT, SIGTERM);
    signals.async_wait([this](auto, auto) {
        terminate();
    });

    SPDLOG_INFO("Game world is running...");
    ctx_.run();
}

void GameWorld::terminate() {
    if (!ctx_.stopped()) {
        guard_.reset();
        ctx_.stop();
    }

    pool_.stop();

    for (const auto &it: ordered_ | std::views::reverse) {
        it->stop();
        SPDLOG_INFO("Stopped module: {}", it->getModuleName());
    }

    SPDLOG_INFO("Game world terminated successfully");
}

asio::io_context &GameWorld::getIOContext() {
    return ctx_;
}

asio::io_context &GameWorld::getWorkerIOContext() {
    return pool_.getIOContext();
}
