#pragma once

#include "database.export.h"

#include <actor/ServerModule.h>

#include <asio/any_io_executor.hpp>
#include <functional>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <atomic>

// #include <bsoncxx/builder/basic/document.hpp>
// #include <bsoncxx/json.hpp>
// #include <mongocxx/pool.hpp>

namespace uranus::database {

    using actor::ServerModule;
    //using bsoncxx::document::value;

    class DATABASE_API DatabaseModule final : public ServerModule {

        using ResultCallback = std::function<void(const std::string &)>;

        // struct TaskNode {
        //     std::string collection;
        //     value query;
        //     ResultCallback cb;
        // };

        using Task = std::function<void()>;
        using TaskQueue = std::queue<Task>;

    public:

        explicit DatabaseModule(asio::any_io_executor exec);
        ~DatabaseModule() override;

        SERVER_MODULE_NAME(DatabaseModule);
        DISABLE_COPY_MOVE(DatabaseModule)

        void start() override;
        void stop() override;

        void query(const std::string &table, const std::string &cond, const ResultCallback &cb);

        void queryPlayer(int64_t pid, const ResultCallback &cb);

    private:
        // std::thread th_;
        asio::any_io_executor exec_;

        TaskQueue queue_;
        std::mutex mtx_;
        std::condition_variable cv_;
        std::atomic_flag stopped_;

        // mongocxx::pool pool_;
    };
}