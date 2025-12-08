#include "ConnectionPipeline.h"
#include "ConnectionPipelineContext.h"
#include "ConnectionInboundHandler.h"
#include "ConnectionOutboundHandler.h"

namespace uranus::network {
    // ConnectionPipeline::ConnectionPipeline(Connection &conn)
    //     : conn_(conn) {
    // }
    //
    // ConnectionPipeline::~ConnectionPipeline() {
    // }
    //
    // Connection &ConnectionPipeline::getConnection() const {
    //     return conn_;
    // }
    //
    // ConnectionPipeline &ConnectionPipeline::addLast(ConnectionHandler *handler) {
    //     handlers_.emplace_back(handler);
    //     return *this;
    // }
    //
    // void ConnectionPipeline::onConnect() {
    //     auto [idx, handler] = getNextInboundHandler();
    //     if (handler == nullptr)
    //         return;
    //
    //     handler->onConnect(ConnectionPipelineContext(this, idx));
    // }
    //
    // void ConnectionPipeline::onDisconnect() {
    //     auto [idx, handler] = getNextInboundHandler();
    //     if (handler == nullptr)
    //         return;
    //
    //     handler->onDisconnect(ConnectionPipelineContext(this, idx));
    // }
    //
    // awaitable<void> ConnectionPipeline::onReceive(MessageHandle &&msg) {
    //     auto [idx, handler] = getNextInboundHandler();
    //     if (handler == nullptr)
    //         co_return;
    //
    //     co_await handler->onReceive(ConnectionPipelineContext(this, idx), std::move(msg));
    // }
    //
    // awaitable<MessageHandle> ConnectionPipeline::beforeSend(MessageHandle &&msg) {
    //     auto [idx, handler] = getPreviousOutboundHandler();
    //     if (handler == nullptr)
    //         co_return std::move(msg);
    //
    //     auto output = co_await handler->beforeSend(ConnectionPipelineContext(this, idx), std::move(msg));
    //     co_return std::move(output);
    // }
    //
    // awaitable<void> ConnectionPipeline::afterSend(MessageHandle &&msg) {
    //     auto [idx, handler] = getPreviousOutboundHandler();
    //     if (handler == nullptr)
    //         co_return;
    //
    //     co_await handler->afterSend(ConnectionPipelineContext(this, idx), std::move(msg));
    // }
    //
    // void ConnectionPipeline::onError(std::error_code ec) {
    //     auto [idx, handler] = getNextInboundHandler();
    //     if (handler == nullptr)
    //         return;
    //
    //     handler->onError(ConnectionPipelineContext(this, idx), ec);
    // }
    //
    // void ConnectionPipeline::onException(const std::exception &e) {
    //     auto [idx, handler] = getNextInboundHandler();
    //     if (handler == nullptr)
    //         return;
    //
    //     handler->onException(ConnectionPipelineContext(this, idx), e);
    // }
    //
    // tuple<size_t, ConnectionInboundHandler *> ConnectionPipeline::getNextInboundHandler() const {
    //     if (handlers_.empty())
    //         return make_tuple(0, nullptr);
    //
    //     for (size_t idx = 0; idx < handlers_.size(); ++idx) {
    //         if (auto *temp = handlers_[idx].get(); temp->type() == ConnectionHandler::Type::kInbound) {
    //             return make_tuple(idx, dynamic_cast<ConnectionInboundHandler *>(temp));
    //         }
    //     }
    //
    //     return make_tuple(0, nullptr);
    // }
    //
    // tuple<size_t, ConnectionOutboundHandler *> ConnectionPipeline::getPreviousOutboundHandler() const {
    //     if (handlers_.empty())
    //         return make_tuple(0, nullptr);
    //
    //     // Deal with while idx == 0
    //     for (size_t idx = handlers_.size(); idx-- > 0;) {
    //         if (auto *temp = handlers_[idx].get(); temp->type() == ConnectionHandler::Type::kOutbound) {
    //             return make_tuple(idx, dynamic_cast<ConnectionOutboundHandler *>(temp));
    //         }
    //     }
    //
    //     return make_tuple(0, nullptr);
    // }
}
