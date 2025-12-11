#include "HttpServerCodec.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "WebSocketFrame.h"
#include "HttpUtils.h"
#include "StatusCode.h"

#include <spdlog/spdlog.h>
#include <algorithm>
#include <unordered_set>
#include <mimalloc.h>
#include <asio.hpp>


namespace uranus::http {

    using std::make_tuple;
    using std::string_view;

    HttpServerCodec::HttpServerCodec(Connection &conn)
        : MessageCodec(conn),
          dynBuf_(asio::dynamic_buffer(buffer_)),
          useWebSocket_(false) {

    }

    HttpServerCodec::~HttpServerCodec() = default;

    awaitable<error_code> HttpServerCodec::encode(Message *msg) {
        if (msg == nullptr)
            co_return CodecErrorCode::kMessageNull;

        auto &socket = getSocket();

        // 写出WebSocket帧
        if (auto *frame = dynamic_cast<WebSocketFrame *>(msg)) {
            const auto payloadLength = frame->payload_.size();

            // 小数据不分包
            if (payloadLength < 8192ul) {
                std::vector<uint8_t, BufferAllocator<uint8_t>> ctrl;
                ctrl.reserve(10);

                uint8_t b0 = 0x80 | (frame->opcode_ &0x0f);
                ctrl.push_back(b0);

                if (payloadLength < 126) {
                    ctrl.push_back(static_cast<uint8_t>(payloadLength));
                } else if (payloadLength <= 0xffff) {
                    ctrl.push_back(126);
                    uint16_t ext = htons(static_cast<uint16_t>(payloadLength));
                    ctrl.insert(ctrl.end(), reinterpret_cast<uint8_t *>(&ext), reinterpret_cast<uint8_t *>(&ext) + 2);
                } else {
                    ctrl.push_back(127);
                    uint64_t ext = htonll(static_cast<uint64_t>(payloadLength));
                    ctrl.insert(ctrl.end(), reinterpret_cast<uint8_t *>(&ext), reinterpret_cast<uint8_t *>(&ext) + 8);
                }

                if (payloadLength == 0) {
                    const auto [ec, len] = co_await asio::async_write(socket, asio::buffer(ctrl));
                    if (ec)
                        co_return ec;
                } else {
                    std::array<asio::const_buffer, 2> buffers = {
                        asio::buffer(ctrl),
                        asio::buffer(frame->payload_)
                    };

                    const auto [ec, _] = co_await asio::async_write(socket, buffers);
                    if (ec)
                        co_return ec;
                }
            }
            // 长数据分包多帧发送
            else {
                size_t total = payloadLength;
                size_t offset = 0;

                bool firstFragment = true;
                bool lastFragment = false;

                std::vector<uint8_t, BufferAllocator<uint8_t>> buffer;

                // 分包循环
                while (total > 0 && !lastFragment) {
                    buffer.clear();

                    const size_t send = total > 8192 ? 8192 : total;
                    const size_t start = offset;
                    const size_t end = start + send;

                    offset = end;
                    total -= send;

                    lastFragment = total <= 0;
                    buffer.reserve(10 + send);

                    uint8_t b0 = 0;

                    // 第一帧写入opcode
                    if (firstFragment) {
                        b0 = frame->opcode_ & 0x0f;
                        firstFragment = false;
                    } else {
                        b0 = 0x00;
                    }

                    // 最后一帧设置FIN=1
                    if (lastFragment) {
                        b0 |= 0x80;
                    }

                    buffer.push_back(b0);

                    if (send < 126) {
                        buffer.push_back(static_cast<uint8_t>(send));
                    } else if (send <= 0xffff) {
                        buffer.push_back(126);
                        uint16_t ext = htons(static_cast<uint16_t>(send));
                        buffer.insert(buffer.end(), reinterpret_cast<uint8_t *>(&ext), reinterpret_cast<uint8_t *>(&ext) + 2);
                    } else {
                        buffer.push_back(127);
                        uint64_t ext = htonll(static_cast<uint64_t>(send));
                        buffer.insert(buffer.end(), reinterpret_cast<uint8_t *>(&ext), reinterpret_cast<uint8_t *>(&ext) + 8);
                    }

                    buffer.insert(buffer.end(),
                        frame->payload_.begin() + static_cast<long>(start),
                        frame->payload_.begin() + static_cast<long>(end));

                    const auto [ec, len] = co_await asio::async_write(socket, asio::buffer(buffer));
                    if (ec)
                        co_return ec;
                }
            }

            co_return error_code{};
        }

        // 写出HttpResponse
        if (auto *res = dynamic_cast<HttpResponse *>(msg)) {
            assert(res->status_ != -1);

            if ((!res->body_.empty() || res->contentLength_ > 0) &&
                !res->hasHeader("Content-Type")) {
                res->setHeader("Content-Type", "text/plain");
            }

            // 判断是否允许带响应体（1xx, 204, 304 以及某些 HEAD 响应不应携带实体）
            const bool bodyPermitted = !((100 <= res->status_ && res->status_ <= 199) || res->status_ == 204 || res->status_ == 304);

            // 是否使用 chunked 传输
            auto te = res->getHeader("Transfer-Encoding");
            bool hasChunked = false;

            if (!te.empty() && detail::case_ignore::equal(te, "chunked")) {
                hasChunked = true;
            }

            // 如果没有 Content-Length 且未声明 TE，则尽可能设置一个 Content-Length
            const bool hasContentLengthHeader = res->hasHeader("Content-Length");
            const size_t bodySize = res->body_.size();

            if (bodyPermitted && !hasChunked && !hasContentLengthHeader) {
                if (const size_t len = res->contentLength_ > 0 ? res->contentLength_ : bodySize; len > 0) {
                    res->setHeader("Content-Length", std::to_string(len));
                }
            }

            // Response line
            BufferString str = "HTTP/1.1 ";
            str += std::to_string(res->status_);
            str += " ";
            str += StatusMessage(res->status_);
            str += "\r\n";

            // 若未设置 Date 头，则自动补充
            if (!res->hasHeader("Date")) {
                res->setHeader("Date", MakeHttpDate());
            }

            // Response Header
            for (const auto &[key, val]: res->headers_) {
                str += key;
                str += ": ";
                str += val;
                str += "\r\n";
            }

            // Header end
            str += "\r\n";

            // 写出响应体
            if (bodyPermitted) {
                if (hasChunked) {
                    // 以 chunked 方式写出单个或多个分块（此处仅根据已聚合的 body_ 输出一个分块）
                    if (!res->body_.empty()) {
                        auto hex = http::detail::FromIntToHex(res->body_.size());
                        str += hex;
                        str += "\r\n";
                        str += res->body_;
                        str += "\r\n";
                    }
                    // 结束块
                    str += "0\r\n";
                    // 写出 trailer（如果有）
                    if (!res->trailers_.empty()) {
                        for (const auto &[k, v]: res->trailers_) {
                            str += k;
                            str += ": ";
                            str += v;
                            str += "\r\n";
                        }
                    }
                    // 结束空行
                    str += "\r\n";
                } else {
                    // 非 chunked：直接写原始 body（Content-Length 已在上文尽量设置）
                    if (!res->body_.empty()) {
                        str += res->body_;
                    }
                }
            }

            const auto [ec, len] = co_await asio::async_write(socket, asio::buffer(str));
            if (ec)
                co_return ec;

            co_return error_code{};
        }

        co_return CodecErrorCode::kTypeNotSupported;
    }

    awaitable<tuple<error_code, MessageHandle>> HttpServerCodec::decode() {
        auto &socket = getSocket();

        if (useWebSocket_) {
            auto frame = WebSocketFrame::getHandle();

            WebSocketControl control{};
            uint8_t header[2];
            std::vector<uint8_t, BufferAllocator<uint8_t>> payload;

            bool firstFragment = true;

            do {
                control.reset();
                memset(header, 0, sizeof(header));
                payload.clear();

                const auto [headerEc, headerLen] = co_await asio::async_read(socket, asio::buffer(header));

                if (headerEc)
                    co_return make_tuple(headerEc, nullptr);

                if (headerLen == 0)
                    co_return make_tuple(CodecErrorCode::kLengthEqualZero, nullptr);

                // 读取帧头部
                control.finish = (header[0] & 0x80) != 0;
                control.opcode = header[0] & 0x0F;
                control.masked = (header[1] & 0x80) != 0;

                // 客户端发来的帧必须带掩码
                if (!control.masked) {
                    co_return make_tuple(CodecErrorCode::kFrameNotMasked, nullptr);
                }

                // 解析帧长度
                if (const uint8_t len7 = header[1] & 0x7F; len7 < 126) {
                    control.length = len7;
                } else if (len7 == 126) {
                    uint16_t ext{};

                    const auto [ec, len] = co_await asio::async_read(socket, asio::buffer(&ext, 2));
                    if (ec)
                        co_return make_tuple(ec, nullptr);
                    if (len == 0)
                        co_return make_tuple(CodecErrorCode::kLengthEqualZero, nullptr);

                    control.length = ntohs(ext);
                } else if (len7 == 127) {
                    uint64_t ext{};

                    const auto [ec, len] = co_await asio::async_read(socket, asio::buffer(&ext, 8));
                    if (ec)
                        co_return make_tuple(ec, nullptr);
                    if (len == 0)
                        co_return make_tuple(CodecErrorCode::kLengthEqualZero, nullptr);

                    control.length = ntohll(ext);
                }

                // 读取掩码 mask-key（客户端->服务器必须有）
                const auto [maskKeyEc, maskKeyLength] = co_await asio::async_read(socket, asio::buffer(control.maskKey));
                if (maskKeyEc)
                    co_return make_tuple(maskKeyEc, nullptr);
                if (maskKeyLength == 0)
                    co_return make_tuple(CodecErrorCode::kLengthEqualZero, nullptr);

                // 读取数据
                if (control.length > 0) {
                    payload.resize(control.length);

                    const auto [payloadEc, payloadLen] = co_await asio::async_read(socket, asio::buffer(payload));
                    if (payloadEc)
                        co_return make_tuple(payloadEc, nullptr);
                    if (payloadLen == 0)
                        co_return make_tuple(CodecErrorCode::kLengthEqualZero, nullptr);

                    // 应用掩码
                    for (size_t idx = 0; idx < payload.size(); idx++) {
                        payload[idx] ^= control.maskKey[idx % 4];
                    }
                }

                // 首片记录 opcode；后续分片必须为 0x0（Continuation）
                if (firstFragment) {
                    frame->opcode_ = control.opcode;
                    firstFragment = false;
                } else if (control.opcode != 0x0) {
                    // 分片续帧的 opcode 必须为 0x0
                    co_return make_tuple(CodecErrorCode::kOpcodeNotContinuation, nullptr);
                }

                frame->payload_.insert_range(frame->payload_.end(), payload);
            } while (!control.finish);

            co_return make_tuple(error_code{}, std::move(frame));
        }

        auto req = HttpRequest::getHandle();

        // 读取HttpRequest头部数据
        {
            const auto [headerEc, headerLen] = co_await asio::async_read_until(socket, dynBuf_, "\r\n\r\n");
            if (headerEc)
                co_return make_tuple(headerEc, nullptr);
            if (headerLen == 0)
                co_return make_tuple(CodecErrorCode::kLengthEqualZero, nullptr);

            // 用string_view作为载体
            const string_view data{reinterpret_cast<const char *>(buffer_.data()), dynBuf_.size()};

            size_t start = 0;
            size_t lineCount = 0;

            while (true) {
                const auto pos = data.find("\r\n", start);
                if (pos == std::string_view::npos)
                    break;

                auto line = data.substr(start, pos - start);
                start = pos + 2;

                if (line.empty())
                    break;

                // 解析请求行
                if (lineCount == 0) {
                    size_t count = 0;
                    auto *ptr = req.get();
                    utils::SplitString(line, ' ', [&](const auto part) {
                        switch (count) {
                            case 0: ptr->method_ = http::ParseMethod(part); break;
                            case 1: ptr->target_ = part; break;
                            case 2: ptr->version_ = part; break;
                            default: break;
                        }
                        ++count;
                    });

                    if (count != 3) {
                        dynBuf_.consume(headerLen);
                        co_return make_tuple(StatusCode::kBadRequest, std::move(req));
                    }

                    if (req->method_ == -1) {
                        dynBuf_.consume(headerLen);
                        co_return make_tuple(StatusCode::kBadRequest, std::move(req));
                    }

                    // Skip URL fragment
                    if (const auto p = req->target_.find('#'); p != string_view::npos) {
                        req->target_.erase(p);
                    }

                    utils::DivideString(req->target_, '?', [&](const string_view lhs, const string_view rhs) {
                        req->path_ = DecodePathComponent(lhs);
                        ParseQueryText(rhs, req->params_);
                    });

                    ++lineCount;
                    continue;
                }

                // Parse headers
                const auto ret = ParseHeader(line, [&](string_view key, string_view value) {
                    req->headers_.emplace(key, value);
                });

                if (!ret) {
                    dynBuf_.consume(headerLen);
                    co_return make_tuple(StatusCode::kBadRequest, std::move(req));
                }

                ++lineCount;
            }

            // Header has already parsed, clear the buffer
            dynBuf_.consume(headerLen);
        }

        if (req->target_.size() >= 8192) {
            co_return make_tuple(StatusCode::kUriTooLong, std::move(req));
        }

        if (req->hasHeader("Accept")) {
            const auto &accept = req->getHeader("Accept");
            const auto ret = ParseAcceptHeader(accept, [&](const std::string &type) {
                req->acceptContentType_.emplace_back(type);
            });

            if (!ret) {
                co_return make_tuple(StatusCode::kBadRequest, std::move(req));
            }
        }

        if (req->hasHeader("Range")) {
            const auto &val = req->getHeader("Range");
            const auto ret = ParseRangeHeader(val, [&](long long first, long long last) {
                req->ranges_.emplace_back(first, last);
            });

            if (!ret) {
                co_return make_tuple(StatusCode::kRangeNotSatisfiable, std::move(req));
            }
        }

        co_return make_tuple(error_code{}, std::move(req));
    }

    awaitable<error_code> HttpServerCodec::readChunked(
        HttpRequest *req,
        const ChunkedHandler &handler,
        const size_t limited)
    {
        if (req == nullptr)
            co_return error_code{};

        const auto te = req->getHeader("Transfer-Encoding");
        if (te.empty())
            co_return error_code{};

        if (!detail::case_ignore::equal(te, "chunked"))
            co_return error_code{};

        static const detail::case_ignore::unordered_set kProhibitedTrailers = {
            // Message framing
            "transfer-encoding", "content-length",

            // Routing
            "host",

            // Authentication
            "authorization", "www-authenticate", "proxy-authenticate",
            "proxy-authorization", "cookie", "set-cookie",

            // Request modifiers
            "cache-control", "expect", "max-forwards", "pragma", "range", "te",

            // Response control
            "age", "expires", "date", "location", "retry-after", "vary", "warning",

            // Payload processing
            "content-encoding", "content-type", "content-range", "trailer"
        };

        detail::case_ignore::unordered_set declaredTrailers;

        if (const auto trailer = req->getHeader("Trailer"); !trailer.empty()) {
            utils::SplitString(trailer, ',', [&](const string_view part) {
                if (!part.empty() && !kProhibitedTrailers.contains(part)) {
                    declaredTrailers.emplace(part);
                }
            });
        }

        auto &socket = getSocket();

        while (true) {
            const auto [chunkedSizeEc, chunkedSizeLength] = co_await asio::async_read_until(socket, dynBuf_, "\r\n");
            if (chunkedSizeEc) {
                co_return chunkedSizeEc;
            }

            // Read the chunked length in hex
            char *endp;
            errno = 0;
            const unsigned long length = std::strtoul(reinterpret_cast<char const *>(buffer_.data()), &endp, 16);
            if (endp == reinterpret_cast<char const *>(buffer_.data()) || errno == ERANGE) {
                dynBuf_.consume(chunkedSizeLength);
                co_return StatusCode::kBadRequest;
            }

            dynBuf_.consume(chunkedSizeLength);

            if (length == 0) {
                // Read and discard trailers until an empty line
                while (true) {
                    const auto [ec, len] = co_await asio::async_read_until(socket, dynBuf_, "\r\n");
                    if (ec) {
                        co_return ec;
                    }

                    std::string_view sv{reinterpret_cast<const char *>(buffer_.data()), dynBuf_.size()};
                    const auto pos = sv.find("\r\n");

                    // Empty line -> end of trailers
                    if (pos == 0) {
                        // len至少为2
                        dynBuf_.consume(len);
                        break;
                    }

                    // Parse the trailers
                    ParseHeader(sv.substr(0, pos), [&](string_view key, string_view value) {
                        if (declaredTrailers.contains(key)) {
                            req->tailers_.emplace(key, value);
                        }
                    });
                    dynBuf_.consume(len);
                }

                // End of chunked body
                break;
            }

            // Read exactly chunked length bytes and append to body
            size_t need = length;

            while (need > 0) {
                const size_t take = std::min(limited, need);

                // 如果缓冲中已有足够数据，则不再读取；否则只补齐差额
                if (dynBuf_.size() < take) {
                    const size_t remaining = take - dynBuf_.size();
                    const auto [ec, _] = co_await asio::async_read(socket, dynBuf_, asio::transfer_exactly(remaining));
                    if (ec) {
                        co_return ec;
                    }
                }

                // 直接从缓冲取出 take 字节
                handler(std::string_view{reinterpret_cast<const char *>(buffer_.data()), take});
                dynBuf_.consume(take);

                need -= take;
            }

            // Read the trailer CRLF
            const auto [crlfEc, crlfLength] = co_await asio::async_read_until(socket, dynBuf_, "\r\n");
            if (crlfEc) {
                co_return crlfEc;
            }

            assert(buffer_.size() >= 2);
            if (!(buffer_[0] == '\r' && buffer_[1] == '\n')) {
                dynBuf_.consume(crlfLength);
                co_return StatusCode::kBadRequest;
            }

            dynBuf_.consume(crlfLength);
        }

        co_return error_code{};
    }

    awaitable<error_code> HttpServerCodec::readContent(HttpRequest *req, const ContentHandler &handler, size_t limited) {
        if (req == nullptr)
            co_return error_code{};

        const auto length = req->getHeaderValueAsULongLong("Content-Length");
        if (length == 0)
            co_return error_code{};

        auto &socket = getSocket();

        req->body_.clear();
        size_t need = length;

        while (need > 0) {
            const size_t take = std::min(limited, need);

            // 如果缓冲中已有足够数据，则不再读取；否则只补齐差额
            if (dynBuf_.size() < take) {
                const size_t remaining = take - dynBuf_.size();
                const auto [ec, _] = co_await asio::async_read(socket, dynBuf_, asio::transfer_exactly(remaining));
                if (ec) {
                    co_return ec;
                }
            }

            // 读取Buffer中的数据
            handler(std::string_view{reinterpret_cast<const char *>(buffer_.data()), take});
            dynBuf_.consume(take);

            need -= take;
        }

        // Parse the Form (url-encoded)
        if (const auto type = req->getHeader("Content-Type"); !type.find("application/x-www-form-urlencoded")) {
            if (req->body_.size() > 8912ull) {
                co_return StatusCode::kPayloadTooLarge;
            }
            ParseQueryText(req->body_, req->params_);
        }

        co_return error_code{};
    }

    void HttpServerCodec::switchToWebSocket() {
        useWebSocket_ = true;
    }

    bool HttpServerCodec::isWebSocketMode() const noexcept {
        return useWebSocket_;
    }
}
