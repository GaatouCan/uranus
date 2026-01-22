#include "DatabaseModule.h"


namespace uranus::database {
    DatabaseModule::DatabaseModule() {
        th_ = std::thread([this]() {
            while (true) {
                std::unique_lock lock(mtx_);
                cv_.wait(lock, [this] {
                    return !queue_.empty() || stopped_.test();
                });

                if (queue_.empty())
                    break;

                auto task = std::move(queue_.front());
                queue_.pop();

                // TODO: Handle task;
            }
        });
    }

    DatabaseModule::~DatabaseModule() {
        if (th_.joinable()) {
            th_.join();
        }
    }

    void DatabaseModule::start() {

    }

    void DatabaseModule::stop() {
        stopped_.test_and_set();
        cv_.notify_all();
    }

    void DatabaseModule::query(const std::string &table, const std::string &cond, const ResultCallback &cb) {
        // TODO
    }
}
