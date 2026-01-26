#include "DatabaseModule.h"


namespace uranus::database {

    // using bsoncxx::builder::basic::kvp;
    // using bsoncxx::builder::basic::make_document;
    // using bsoncxx::document::value;

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

                // auto client = pool_.acquire();
                // auto db = (*client)["admin"];
                //
                // auto collection = db[task.collection];
                // auto res = collection.find_one(task.query.view());
                //
                // if (res.has_value()) {
                //     std::invoke(task.cb, bsoncxx::to_json(res.value()));
                // }
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

    void DatabaseModule::queryPlayer(int64_t pid, const ResultCallback &cb) {
        // TODO
    }
}
