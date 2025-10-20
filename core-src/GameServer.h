#pragma once

#include "ServerModule.h"

#include <unordered_map>
#include <typeindex>
#include <vector>
#include <memory>
#include <asio/io_context.hpp>


namespace uranus {

    using std::unordered_map;
    using std::vector;
    using std::unique_ptr;
    using std::type_index;
    using std::make_unique;

    class SingleIOContextPool;
    class MultiIOContextPool;

    class CORE_API GameServer {

    public:
        GameServer();
        virtual ~GameServer();

        DISABLE_COPY_MOVE(GameServer)

        virtual void Start();
        virtual void Stop();

        template<class T, class... Args>
            requires std::is_base_of_v<ServerModule, T>
        T *CreateModule(Args &&... args);

        template<class T>
            requires std::is_base_of_v<ServerModule, T>
        T *GetModule() const;

        asio::io_context &GetMainIOContext();

        [[nodiscard]] asio::io_context &GetSocketIOContext() const;
        [[nodiscard]] asio::io_context &GetWorkerIOContext() const;

    private:
        asio::io_context ctx_;

        unordered_map<type_index, unique_ptr<ServerModule> > modules_;
        vector<ServerModule *> ordered_;

        unique_ptr<MultiIOContextPool> io_pool_;
        unique_ptr<SingleIOContextPool> worker_pool_;
    };

    template<class T, class... Args>
        requires std::is_base_of_v<ServerModule, T>
    T *GameServer::CreateModule(Args &&... args) {
        if (const auto iter = modules_.find(typeid(T)); iter != modules_.end()) {
            return dynamic_cast<T *>(iter->second.get());
        }

        auto *ptr = new T(this, std::forward<Args>(args)...);

        modules_.insert_or_assign(typeid(T), unique_ptr<ServerModule>(ptr));
        ordered_.emplace_back(ptr);

        return ptr;
    }

    template<class T>
        requires std::is_base_of_v<ServerModule, T>
    T *GameServer::GetModule() const {
        if (const auto iter = modules_.find(typeid(T)); iter != modules_.end()) {
            return dynamic_cast<T *>(iter->second.get());
        }
        return nullptr;
    }
}
