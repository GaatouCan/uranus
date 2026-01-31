#pragma once

#include <base/SingleIOContextPool.h>
#include <actor/ServerModule.h>

#include <memory>
#include <vector>
#include <unordered_map>

namespace uranus {

    using std::unique_ptr;
    using std::make_unique;
    using std::vector;
    using std::unordered_map;

    using actor::ServerModule;

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

        template<typename T, typename... Args>
        requires std::derived_from<T, ServerModule>
        void pushModule(Args &&...args);

        [[nodiscard]] ServerModule *getModule(const std::string &name) const;

    private:
        asio::io_context ctx_;
        asio::executor_work_guard<asio::io_context::executor_type> guard_;

        SingleIOContextPool pool_;

        unordered_map<std::string, unique_ptr<ServerModule>> modules_;
        vector<ServerModule *> ordered_;
    };

    template<typename T, typename ... Args>
    requires std::derived_from<T, ServerModule>
    void GameWorld::pushModule(Args &&...args) {
        auto unique = make_unique<T>(std::forward<Args>(args)...);

        const auto name = unique->getModuleName();
        if (modules_.contains(name)) {
            throw std::logic_error(std::format("ServerModule[{}] already exists!", name));
        }

        auto *raw = unique.get();

        modules_.insert_or_assign(name, std::move(unique));
        ordered_.emplace_back(raw);
    }

#define GET_MODULE(gw, s) \
    dynamic_cast<s *>((gw)->getModule(#s))

}