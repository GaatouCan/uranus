#pragma once

#include <base/SingleIOContextPool.h>
#include <memory>
#include <vector>
#include <unordered_map>

namespace uranus {

    namespace actor {
        class ServerModule;
    }

    using actor::ServerModule;
    using std::unique_ptr;
    using std::vector;
    using std::unordered_map;

    class GameWorld final {

    public:
        GameWorld();
        ~GameWorld();

        DISABLE_COPY_MOVE(GameWorld)

        void run();
        void terminate();

        [[nodiscard]] bool isRunning() const;

        asio::io_context &getIOContext();
        asio::io_context &getWorkerIOContext();

        void pushServerModule(ServerModule *module);
        [[nodiscard]] ServerModule *getServerModule(const std::string &name) const;

    private:
        asio::io_context ctx_;
        asio::executor_work_guard<asio::io_context::executor_type> guard_;

        SingleIOContextPool pool_;

        unordered_map<std::string, unique_ptr<ServerModule>> modules_;
        vector<ServerModule *> ordered_;
    };
}