#pragma once

#include "Message.h"
#include "noncopy.h"

#include <memory>
#include <vector>
#include <tuple>

namespace uranus::network {

    using std::vector;
    using std::tuple;
    using std::unique_ptr;
    using std::make_tuple;
    using std::make_unique;

    class Connection;
    class ConnectionHandler;
    class ConnectionInboundHandler;
    class ConnectionOutboundHandler;
    class ConnectionPipelineContext;

    class BASE_API ConnectionPipeline final {

        friend class ConnectionPipelineContext;

    public:
        ConnectionPipeline() = delete;

        explicit ConnectionPipeline(Connection &conn);
        ~ConnectionPipeline();

        DISABLE_COPY_MOVE(ConnectionPipeline)

        [[nodiscard]] Connection &getConnection() const;

        void onReceive(MessageHandle &&msg);

    private:
        [[nodiscard]] tuple<size_t, ConnectionInboundHandler *> getNextInboundHandler() const;
        [[nodiscard]] tuple<size_t, ConnectionOutboundHandler *> getPreviousOutboundHandler() const;

    private:
        Connection &conn_;
        vector<unique_ptr<ConnectionHandler>> handlers_;
    };

    class BASE_API ConnectionPipelineContext final {

    public:
        ConnectionPipelineContext() = delete;

        ConnectionPipelineContext(ConnectionPipeline *pipeline, size_t idx);
        ~ConnectionPipelineContext();

        ConnectionPipelineContext(const ConnectionPipelineContext &rhs);
        ConnectionPipelineContext &operator=(const ConnectionPipelineContext &rhs);

        ConnectionPipelineContext(ConnectionPipelineContext &&rhs) noexcept;
        ConnectionPipelineContext &operator=(ConnectionPipelineContext &&rhs) noexcept;


    private:
        ConnectionPipeline *pipeline_;
        size_t index_;
    };
}
