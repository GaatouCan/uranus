#pragma once

#include "base/MultiIOContextPool.h"
#include "base/types.h"

#include <shared_mutex>
#include <unordered_map>
#include <memory>
#include <functional>

namespace uranus::network {

    class BaseConnection;

    using asio::awaitable;
    using std::shared_ptr;
    using std::shared_mutex;
    using std::unique_lock;
    using std::shared_lock;
    using std::unordered_map;

    class BASE_API ServerBootstrap final {

        using InitialCallback   = std::function<shared_ptr<BaseConnection>(ServerBootstrap &, TcpSocket &&)>;
        using RemoveCallback    = std::function<void(const std::string &)>;
        using ExceptionCallback = std::function<void(std::exception &e)>;

    public:
        ServerBootstrap();
        ~ServerBootstrap();

        DISABLE_COPY_MOVE(ServerBootstrap)

#ifdef URANUS_SSL
        void useCertificateChainFile(const std::string &filename);
        void usePrivateKeyFile(const std::string &filename);
#endif

        void run(int num, uint16_t port);
        void terminate();

        [[nodiscard]] shared_ptr<BaseConnection> find(const std::string &key) const;
        void remove(const std::string &key);

        void onInitial(const InitialCallback &cb);
        void onRemove(const RemoveCallback &cb);
        void onException(const ExceptionCallback &cb);

    private:
       awaitable<void> waitForClient(uint16_t port);

    private:
        asio::io_context ctx_;
        asio::executor_work_guard<asio::io_context::executor_type> guard_;

        TcpAcceptor acceptor_;

#ifdef URANUS_SSL
        asio::ssl::context sslContext_;
#endif

        MultiIOContextPool pool_;

        mutable shared_mutex mutex_;
        unordered_map<std::string, shared_ptr<BaseConnection>> conns_;

        InitialCallback onInit_;
        RemoveCallback onRemove_;
        ExceptionCallback onException_;

    };
}