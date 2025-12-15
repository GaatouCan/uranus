#pragma once

#include <base/SingleIOContextPool.h>
#include <actor/ServerModule.h>


namespace uranus {

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

    private:
        asio::io_context ctx_;
        asio::executor_work_guard<asio::io_context::executor_type> guard_;

        SingleIOContextPool pool_;
    };
}