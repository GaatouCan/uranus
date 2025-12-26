#pragma once

#include "Package.h"

#include <base/noncopy.h>
#include <base/types.h>
#include <base/AttributeMap.h>
#include <base/IdentAllocator.h>

#include <asio/co_spawn.hpp>
#include <asio/any_completion_handler.hpp>
#include <map>
#include <memory>
#include <functional>


namespace uranus::actor {

    class BaseActor;
    class ServerModule;

    using std::unique_ptr;
    using std::function;
    using asio::awaitable;
    using asio::co_spawn;

    using ActorDeleter = function<void(BaseActor *)>;
    using ActorHandle = unique_ptr<BaseActor, ActorDeleter>;


    /// Actor上下文，负责管理Actor生命周期、消息分发及RPC会话
    class ACTOR_API ActorContext : public std::enable_shared_from_this<ActorContext> {

        using SessionHandle = asio::any_completion_handler<void(PackageHandle)>;

        /// 会话节点，用于异步调用（RPC）的回调管理
        struct ACTOR_API SessionNode {
            /** 完成处理器 **/
            asio::any_completion_handler<void(PackageHandle)> handle;

            /** 执行器工作保护 **/
            asio::executor_work_guard<asio::any_completion_executor> work;

            /** 会话ID **/
            uint32_t sess;

            SessionNode() = delete;
            SessionNode(SessionHandle &&h, uint32_t s);
        };

    public:
        ActorContext() = delete;

        explicit ActorContext(asio::io_context &ctx);
        virtual ~ActorContext();

        DISABLE_COPY_MOVE(ActorContext)

        void setId(uint32_t id);
        [[nodiscard]] uint32_t getId() const;
        [[nodiscard]] AttributeMap &attr();

        void setUpActor(ActorHandle &&handle);
        [[nodiscard]] BaseActor *getActor() const;

        virtual void run();
        virtual void terminate();

        [[nodiscard]] bool isRunning() const;

        /// 处理由其他Actor发送过来的信封包
        void pushEnvelope(Envelope &&envelope);

#pragma region For inner actor to call
        /// 获取指定名称的服务模块
        virtual ServerModule *getModule(const std::string &name) const = 0;
        /// 获取服务列表
        virtual std::map<std::string, uint32_t> getServiceList() const = 0;

        /// 发送数据包到目标Actor（单向）
        virtual void send(int ty, uint32_t target, PackageHandle &&pkg) = 0;

        /// 异步调用目标Actor并等待返回结果（RPC）
        auto call(int ty, uint32_t target, PackageHandle &&pkg) -> awaitable<PackageHandle>;
#pragma endregion

    protected:
        /// 向目标Actor发送请求
        virtual bool sendRequest(int ty, uint32_t sess, uint32_t target, PackageHandle &&pkg) = 0;

        /// 向请求方返回响应
        virtual void sendResponse(int ty, uint32_t sess, uint32_t target, PackageHandle &&pkg) = 0;

        /// 错误码回调处理
        virtual void onErrorCode(std::error_code ec);

        /// 异常回调处理
        virtual void onException(std::exception &e);

    private:
        /// 协程处理函数，负责处理邮箱中的消息
        awaitable<void> process();

    private:
        /** ASIO IO上下文引用 **/
        asio::io_context &ctx_;

        /** Actor实例句柄 **/
        ActorHandle handle_;

        /** 消息信箱 **/
        ConcurrentChannel<Envelope> mailbox_;

        /** 会话ID分配器 **/
        IdentAllocator<uint32_t, true> sessAlloc_;

        /** 正在进行的会话 **/
        std::unordered_map<uint32_t, unique_ptr<SessionNode>> sessions_;
        std::mutex sessMutex_;

        AttributeMap attr_;

        /** Actor的唯一ID **/
        uint32_t id_;
    };
}
