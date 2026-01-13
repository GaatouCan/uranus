#pragma once

#include "base/base.export.h"
#include "base/noncopy.h"
#include "base/types.h"

#include <memory>
#include <functional>

namespace uranus::network {

    class Connection;

    using asio::awaitable;
    using std::shared_ptr;
    using std::unordered_map;
    using std::thread;
    using std::vector;

    class BASE_API ServerBootstrap final {

        using AcceptCallback    = std::function<shared_ptr<Connection>(TcpSocket &&)>;
        using RemoveCallback    = std::function<void(const std::string &)>;
        using ExceptionCallback = std::function<void(std::exception &e)>;

    public:
        ServerBootstrap();

        explicit ServerBootstrap(unsigned int threads);
        ~ServerBootstrap();

        DISABLE_COPY_MOVE(ServerBootstrap)

#ifdef URANUS_SSL
        void useCertificateChainFile(const std::string &filename);
        void usePrivateKeyFile(const std::string &filename);
#endif

        void runInBlock(uint16_t port, unsigned int threads = std::thread::hardware_concurrency());

        /// The internal io_context not run in the called thread
        void run(uint16_t port, unsigned int threads = std::thread::hardware_concurrency());

        void terminate();

        void onAccept(const AcceptCallback &cb);
        void onRemove(const RemoveCallback &cb);
        void onException(const ExceptionCallback &cb);

    private:
       awaitable<void> waitForClient(uint16_t port);

    private:
        asio::io_context ctx_;
        asio::executor_work_guard<asio::io_context::executor_type> guard_;

#ifdef URANUS_SSL
        asio::ssl::context sslContext_;
#endif

        TcpAcceptor acceptor_;

        vector<thread> pool_;

        AcceptCallback onAccept_;
        RemoveCallback onRemove_;
        ExceptionCallback onException_;

    };
}