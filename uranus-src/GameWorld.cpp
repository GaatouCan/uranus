#include "GameWorld.h"

#include <actor/ServerModule.h>
#include <format>

namespace uranus {
    GameWorld::GameWorld()
        : guard_(asio::make_work_guard(ctx_)) {
    }

    GameWorld::~GameWorld() {
        ordered_.clear();
        modules_.clear();
    }

    void GameWorld::run() {
    }

    void GameWorld::terminate() {
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

    void GameWorld::pushServerModule(ServerModule *module) {
        if (!module)
            return;

        const auto name = module->getModuleName();
        if (modules_.contains(name)) {
            throw std::logic_error(std::format("ServerModule[{}] already exists!", name));
        }

        modules_.insert_or_assign(name, unique_ptr<ServerModule>(module));
        ordered_.emplace_back(module);
    }

    ServerModule *GameWorld::getServerModule(const std::string &name) const {
        const auto iter = modules_.find(name);
        return iter != modules_.end() ? iter->second.get() : nullptr;
    }
}
