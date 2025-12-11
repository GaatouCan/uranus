#pragma once

#include "BufferHeap.h"

#include <network/MessageCodec.h>

namespace uranus::http {
    using network::MessageCodec;
    using network::Connection;

    using asio::awaitable;
    using std::vector;
    using std::tuple;
    using std::error_code;

    using BufferString = std::basic_string<char, std::char_traits<char>, BufferAllocator<char>>;

    class HttpRequest;
    class HttpResponse;

    class HTTP_API HttpServerCodec final : public MessageCodec {

        using ChunkedHandler = std::function<void(std::string_view)>;
        using ContentHandler = std::function<void(std::string_view)>;

    public:
        explicit HttpServerCodec(Connection &conn);
        ~HttpServerCodec() override;

        awaitable<error_code> encode(Message *msg) override;
        awaitable<tuple<error_code, MessageHandle>> decode() override;

        awaitable<error_code> readChunked(HttpRequest *req, const ChunkedHandler &handler, size_t limited = 8192);
        awaitable<error_code> readContent(HttpRequest *req, const ContentHandler &handler, size_t limited = 8192);

        void switchToWebSocket();
        [[nodiscard]] bool isWebSocketMode() const noexcept;

    private:
        vector<uint8_t, BufferAllocator<uint8_t>> buffer_;
        asio::dynamic_vector_buffer<uint8_t, BufferAllocator<uint8_t>> dynBuf_;

        bool useWebSocket_;
    };
}
