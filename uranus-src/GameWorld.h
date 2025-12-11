#pragma once

#include <base/SingleIOContextPool.h>
#include <base/ServerBootstrap.h>
#include <../actor-src/include/actor/ServerModule.h>

#include <vector>
#include <unordered_map>
#include <typeindex>

using uranus::SingleIOContextPool;
using uranus::ServerBootstrap;
using uranus::ServerModule;

class GameWorld final : public ServerBootstrap {

public:
    GameWorld();
    ~GameWorld() override;

    void run() override;
    void terminate() override;

    asio::io_context &getIOContext();
    asio::io_context &getWorkerIOContext();

    template<class T, class... Args>
    requires std::is_base_of_v<ServerModule, T>
    T *createModule(Args &&... args);

    template<class T>
    requires std::is_base_of_v<ServerModule, T>
    T *getModule() const;

private:
    asio::io_context ctx_;
    asio::executor_work_guard<asio::io_context::executor_type> guard_;

    SingleIOContextPool pool_;

    std::unordered_map<std::type_index, std::unique_ptr<ServerModule>> modules_;
    std::vector<ServerModule *> ordered_;
};

template<class T, class ... Args>
requires std::is_base_of_v<ServerModule, T>
T *GameWorld::createModule(Args &&...args) {
    if (const auto it = modules_.find(typeid(T)); it != modules_.end()) {
        return dynamic_cast<T *>(it->second.get());
    }

    auto module = std::make_unique<T>(*this, std::forward<Args>(args)...);
    auto *ptr = module.get();

    modules_.insert_or_assign(typeid(T), std::move(module));
    ordered_.emplace_back(ptr);

    return ptr;
}

template<class T>
requires std::is_base_of_v<ServerModule, T>
T *GameWorld::getModule() const {
    auto it = modules_.find(typeid(T));
    if (it == modules_.end())
        return nullptr;

    return dynamic_cast<T *>(it->second.get());
}
