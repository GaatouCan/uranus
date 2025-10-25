#include "GameServer.h"
#include "base/SingleIOContextPool.h"
#include "base/MultiIOContextPool.h"

#include <asio/signal_set.hpp>
#include <spdlog/spdlog.h>

namespace uranus {
    GameServer::GameServer()
        : io_num_(4),
          worker_num_(8) {
        io_pool_ = make_unique<MultiIOContextPool>();
        worker_pool_ = make_unique<SingleIOContextPool>();
    }

    GameServer::~GameServer() {
    }

    void GameServer::Start() {
        for (const auto &module: ordered_) {
            SPDLOG_INFO("Start module[{}]", module->GetModuleName());
            module->Start();
        }

        SPDLOG_INFO("IO run with {} threads", io_num_);
        io_pool_->Start(io_num_);

        SPDLOG_INFO("Worker run with {} threads", worker_num_);
        worker_pool_->Start(worker_num_);

        asio::signal_set signals(ctx_, SIGINT, SIGTERM);
        signals.async_wait([this](auto, auto) {
            Stop();
        });

        SPDLOG_INFO("Game server started");
        ctx_.run();
    }

    void GameServer::Stop() {
        if (ctx_.stopped())
            return;

        ctx_.stop();

        io_pool_->Stop();
        worker_pool_->Stop();

        for (auto iter = ordered_.rbegin(); iter != ordered_.rend(); ++iter) {
            SPDLOG_INFO("Stop module[{}]", (*iter)->GetModuleName());
            (*iter)->Stop();
        }
        ordered_.clear();

        SPDLOG_INFO("Game server shutdown");
    }

    asio::io_context &GameServer::GetMainIOContext() {
        return ctx_;
    }

    bool GameServer::IsRunning() const {
        return !ctx_.stopped();
    }

    asio::io_context &GameServer::GetSocketIOContext() const {
        return io_pool_->GetIOContext();
    }

    asio::io_context &GameServer::GetWorkerIOContext() const {
        return worker_pool_->GetIOContext();
    }
}