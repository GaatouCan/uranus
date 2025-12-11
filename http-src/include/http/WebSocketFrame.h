#pragma once

#include "BufferHeap.h"

#include <base/Message.h>
#include <base/Recycler.h>


namespace uranus::http {

    using std::tuple;
    using std::make_tuple;
    using std::string;
    using std::string_view;
    using std::vector;

    struct HTTP_API WebSocketControl {
        bool        finish;
        uint8_t     opcode;
        bool        masked;
        uint64_t    length;
        uint8_t     maskKey[4];

        void reset() {
            finish = false;
            opcode = 0;
            masked = false;
            length = 0;
            memset(maskKey, 0, sizeof(maskKey));
        }
    };

    class HTTP_API WebSocketFrame final : public Message {

        friend class HttpServerCodec;

    public:
        WebSocketFrame() = delete;

        DECLARE_MESSAGE_POOL_GET(WebSocketFrame)

        explicit WebSocketFrame(const WebSocketFrameRecyclerHandle &handle);
        ~WebSocketFrame() override;

        void recycle();

        /// For Read

        [[nodiscard]] uint8_t getOpcode() const;

        [[nodiscard]] bool isTextFrame() const;
        [[nodiscard]] bool isBinaryFrame() const;

        [[nodiscard]] tuple<const uint8_t *, size_t> getPayload() const;
        [[nodiscard]] string_view getPayloadAsStringView() const;

        /// For Write

        void setOpcode(uint8_t op);

        void setAsTextFrame();
        void setAsBinaryFrame();

        void setData(const char *data, size_t len);
        void setData(string_view data);
        void setData(const string &data);
        void setData(const vector<uint8_t> &data);

    private:
        WebSocketFrameRecyclerHandle handle_;

        uint8_t opcode_;
        vector<uint8_t, BufferAllocator<uint8_t>> payload_;
    };

    DECLARE_MESSAGE_POOL(WebSocketFrame)
}
