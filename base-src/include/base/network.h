#pragma once

#include "types.h"
#include "Message.h"
#include "AttributeMap.h"
#include "MultiIOContextPool.h"

#include <string>
#include <tuple>
#include <shared_mutex>
#include <unordered_map>
#include <asio/co_spawn.hpp>
#include <asio/detached.hpp>


namespace uranus::network {

    using asio::co_spawn;
    using asio::detached;
    using asio::awaitable;

    using std::tuple;
    using std::unordered_map;
    using std::shared_mutex;
    using std::shared_lock;
    using std::unique_lock;
    using std::make_tuple;
    using std::error_code;
    using std::shared_ptr;
    using std::unique_ptr;
    using std::make_shared;
    using std::make_unique;
    using std::enable_shared_from_this;

#pragma region ServerBootStrap

    class Connection;

    class BASE_API ServerBootstrap {

    public:
        ServerBootstrap();
        virtual ~ServerBootstrap();

        DISABLE_COPY_MOVE(ServerBootstrap)

        virtual void run(int num, uint16_t port);
        virtual void terminate();

#ifdef URANUS_SSL
        void useCertificateChainFile(const std::string &filename);
        void usePrivateKeyFile(const std::string &filename);
#endif

        virtual shared_ptr<Connection> find(const std::string &key) = 0;
        virtual void remove(const std::string &key) = 0;

    protected:
        virtual awaitable<void> waitForClient(uint16_t port) = 0;

    protected:
        asio::io_context ctx_;
        asio::executor_work_guard<asio::io_context::executor_type> guard_;

        TcpAcceptor acceptor_;

#ifdef URANUS_SSL
        asio::ssl::context sslContext_;
#endif

        MultiIOContextPool pool_;
    };
#pragma endregion


#pragma region Base Connection
    class BASE_API Connection : public enable_shared_from_this<Connection> {

    public:
        Connection() = delete;

        Connection(ServerBootstrap &server, TcpSocket &&socket);
        virtual ~Connection();

        DISABLE_COPY_MOVE(Connection)

        [[nodiscard]] ServerBootstrap &getServerBootstrap() const;

        TcpSocket &getSocket();

        virtual void connect();
        virtual void disconnect();

        [[nodiscard]] bool isConnected() const;

        [[nodiscard]] const std::string &getKey() const;
        [[nodiscard]] asio::ip::address remoteAddress() const;

        void setExpirationSecond(int sec);

        AttributeMap &attr();

        virtual void sendMessage(MessageHandle &&msg) = 0;
        virtual void sendMessage(Message *msg) = 0;

    protected:
        virtual awaitable<void> readMessage() = 0;
        virtual awaitable<void> writeMessage() = 0;

        virtual void onConnect() = 0;
        virtual void onDisconnect() = 0;

        virtual void onTimeout() = 0;
        virtual void onErrorCode(error_code ec) = 0;
        virtual void onException(std::exception &e) = 0;

    private:
        awaitable<void> watchdog();

    protected:
        ServerBootstrap &server_;
        TcpSocket socket_;

        std::string key_;
        AttributeMap attr_;

        SteadyTimer watchdog_;
        SteadyDuration expiration_;
        SteadyTimePoint received_;
    };
#pragma endregion

#pragma region MessageCodec
    template<class T>
    requires std::is_base_of_v<Message, T>
    class MessageCodec {

    public:
        using MessageType = std::decay_t<T>;
        using MessageHandleType = Message::Pointer<MessageType>;

        MessageCodec() = delete;

        explicit MessageCodec(Connection &conn);
        virtual ~MessageCodec();

        DISABLE_COPY_MOVE(MessageCodec)

        [[nodiscard]] Connection &getConnection() const;
        [[nodiscard]] TcpSocket &getSocket() const;

        virtual awaitable<tuple<error_code, MessageHandleType>> decode() = 0;
        virtual awaitable<error_code> encode(MessageType *msg) = 0;

    private:
        Connection &conn_;
    };
#pragma endregion


    template<class Codec>
    concept kCodecType = requires { typename Codec::MessageType; }
        && std::derived_from<Codec, MessageCodec<typename Codec::MessageType>>;


    template<kCodecType Codec>
    class ConnectionImpl : public Connection {

    public:
        using CodecType = Codec;
        using MessageType = Codec::MessageType;
        using MessageHandleType = Codec::MessageHandleType;

        ConnectionImpl(ServerBootstrap &server, TcpSocket &&socket);
        ~ConnectionImpl() override;

        void connect() override;
        void disconnect() override;

        Codec &codec();

        void sendMessage(MessageHandle &&msg) override;
        void sendMessage(Message *msg) override;

        void send(MessageHandleType &&msg);
        void send(MessageType *msg);

    protected:
        awaitable<void> readMessage() override;
        awaitable<void> writeMessage() override;

        virtual void onReadMessage(MessageHandleType &&msg) = 0;
        virtual void beforeWrite(MessageType *msg) = 0;
        virtual void afterWrite(MessageHandleType &&msg) = 0;

    private:
        Codec codec_;
        ConcurrentChannel<MessageHandleType> output_;
    };

    template<class T>
    concept kConnectionType = requires { typename T::CodecType; }
        && kCodecType<typename T::CodecType>
        && std::derived_from<T, ConnectionImpl<typename T::CodecType>>;


    template<kConnectionType T>
    class ServerBootstrapImpl : public ServerBootstrap {

    public:
        using Pointer = shared_ptr<T>;

        using InitialCallback = std::function<void(const Pointer &)>;
        using RemoveCallback = std::function<void(const std::string &)>;
        using ExceptionCallback = std::function<void(std::exception &e)>;

        ServerBootstrapImpl();
        ~ServerBootstrapImpl() override;

        void terminate() override;

        shared_ptr<Connection> find(const std::string &key) override;
        void remove(const std::string &key) override;

        void onInitial(const InitialCallback &cb);
        void onRemove(const RemoveCallback &cb);

        void onException(const ExceptionCallback &cb);

    protected:
        awaitable<void> waitForClient(uint16_t port) override;

    private:
        mutable shared_mutex mutex_;
        unordered_map<std::string, Pointer> connMap_;

        InitialCallback onInitial_;
        RemoveCallback onRemove_;
        ExceptionCallback onException_;
    };

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::MessageCodec(Connection &conn)
        : conn_(conn) {
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    MessageCodec<T>::~MessageCodec() = default;

    template<class T>
    requires std::is_base_of_v<Message, T>
    Connection &MessageCodec<T>::getConnection() const {
        return conn_;
    }

    template<class T>
    requires std::is_base_of_v<Message, T>
    TcpSocket &MessageCodec<T>::getSocket() const {
        return conn_.getSocket();
    }


    template<kCodecType Codec>
    ConnectionImpl<Codec>::ConnectionImpl(ServerBootstrap &server, TcpSocket &&socket)
        : Connection(server, std::move(socket)),
          codec_(dynamic_cast<Connection &>(*this)),
          output_(socket_.get_executor(), 1024) {
    }

    template<kCodecType Codec>
    ConnectionImpl<Codec>::~ConnectionImpl() = default;

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::connect() {
        Connection::connect();
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::disconnect() {
        if (!isConnected())
            return;

#ifdef URANUS_SSL
        socket_.next_layer().close();
#else
        socket_.close();
#endif
        watchdog_.cancel();

        output_.cancel();
        output_.close();

        if (!attr().has("REPEATED")) {
            server_.remove(key_);
        }

        // Call the virtual method
        onDisconnect();
    }

    template<kCodecType Codec>
    Codec &ConnectionImpl<Codec>::codec() {
        return codec_;
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::sendMessage(MessageHandle &&msg) {
        if (msg == nullptr)
            return;

        auto del = msg.get_deleter();
        auto *ptr = msg.get();

        if (auto *temp = dynamic_cast<MessageType *>(ptr)) {
            MessageHandleType handle{ temp, del };
            this->send(std::move(handle));
            return;
        }

        del(ptr);
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::sendMessage(Message *msg) {
        if (msg == nullptr)
            return;

        if (auto *temp = dynamic_cast<MessageType *>(msg)) {
            this->send(temp);
            return;
        }

        delete msg;
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::send(MessageHandleType &&msg) {
        if (msg == nullptr)
            return;

        if (isConnected() && output_.is_open()) {
            output_.try_send_via_dispatch(error_code{}, std::move(msg));
        }
    }

    template<kCodecType Codec>
    void ConnectionImpl<Codec>::send(MessageType *msg) {
        if (msg == nullptr)
            return;

        MessageHandleType handle{msg, Message::Deleter::make()};
        this->send(std::move(handle));
    }

    template<kCodecType Codec>
    awaitable<void> ConnectionImpl<Codec>::readMessage() {
        try {
            while (isConnected()) {
                auto [ec, msg] = co_await codec_.decode();

                if (ec) {
                    this->onErrorCode(ec);
                    disconnect();
                    break;
                }

                this->onReadMessage(std::move(msg));
            }
        } catch (std::exception &e) {
            this->onException(e);
        }
    }

    template<kCodecType Codec>
    awaitable<void> ConnectionImpl<Codec>::writeMessage() {
        try {
            while (isConnected() && output_.is_open()) {
                auto [ec, msg] = co_await output_.async_receive();

                if (ec == asio::error::operation_aborted ||
                    ec == asio::experimental::error::channel_closed) {
                    break;
                }

                if (ec) {
                    this->onErrorCode(ec);
                    disconnect();
                    break;
                }

                if (msg == nullptr)
                    continue;

                this->beforeWrite(msg.get());

                if (const auto writeEc = co_await codec_.encode(msg.get())) {
                    this->onErrorCode(writeEc);
                    disconnect();
                }

                this->afterWrite(std::move(msg));
            }
        } catch (std::exception &e) {
            this->onException(e);
        }
    }

    template<kConnectionType T>
    ServerBootstrapImpl<T>::ServerBootstrapImpl() = default;

    template<kConnectionType T>
    ServerBootstrapImpl<T>::~ServerBootstrapImpl() {
        ServerBootstrapImpl::terminate();
    }

    template<kConnectionType T>
    void ServerBootstrapImpl<T>::terminate() {
        ServerBootstrap::terminate();
        connMap_.clear();
    }

    template<kConnectionType T>
    shared_ptr<Connection> ServerBootstrapImpl<T>::find(const std::string &key) {
        if (ctx_.stopped())
            return nullptr;

        shared_lock lock(mutex_);
        const auto iter = connMap_.find(key);
        return iter != connMap_.end() ? iter->second : nullptr;
    }

    template<kConnectionType T>
    void ServerBootstrapImpl<T>::remove(const std::string &key) {
        if (ctx_.stopped())
            return;

        {
            unique_lock lock(mutex_);
            connMap_.erase(key);
        }

        if (onRemove_) {
            std::invoke(onRemove_, key);
        }
    }

    template<kConnectionType T>
    void ServerBootstrapImpl<T>::onInitial(const InitialCallback &cb) {
        onInitial_ = cb;
    }

    template<kConnectionType T>
    void ServerBootstrapImpl<T>::onRemove(const RemoveCallback &cb) {
        onRemove_ = cb;
    }

    template<kConnectionType T>
    void ServerBootstrapImpl<T>::onException(const ExceptionCallback &cb) {
        onException_ = cb;
    }

    template<kConnectionType T>
    awaitable<void> ServerBootstrapImpl<T>::waitForClient(uint16_t port) {
        try {
            acceptor_.open(asio::ip::tcp::v4());
            acceptor_.bind({asio::ip::tcp::v4(), port});
            acceptor_.listen(port);

            while (!ctx_.stopped()) {
                auto [ec, socket] = co_await acceptor_.async_accept(pool_.getIOContext());

                if (ec) {
                    continue;
                }

                if (!socket.is_open()) {
                    continue;
                }

#ifdef URANUS_SSL
                const auto conn = std::make_shared<T>(*this, TcpSocket(std::move(socket), sslContext_));
#else
                const auto conn = std::make_shared<ActorConnection>(*this, std::move(socket));
#endif

                const auto key = conn->getKey();

                bool repeated = false;

                do {
                    std::unique_lock lock(mutex_);

                    if (connMap_.contains(key)) {
                        repeated = true;
                        break;
                    }

                    connMap_.insert_or_assign(key, conn);
                } while (false);

                if (repeated) {
                    conn->disconnect();
                    conn->attr().set("REPEATED", 1);
                    continue;
                }

                if (onInitial_) {
                    std::invoke(onInitial_, conn);
                }

                conn->connect();
            }
        } catch (std::exception &e) {
            if (onException_) {
                std::invoke(onException_, e);
            }
        }
    }
}
