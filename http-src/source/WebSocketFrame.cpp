#include "WebSocketFrame.h"

namespace uranus::http {
    WebSocketFrame::WebSocketFrame(const WebSocketFrameRecyclerHandle &handle)
        : handle_(handle),
          opcode_(0) {
    }

    WebSocketFrame::~WebSocketFrame() {
    }

    void WebSocketFrame::recycle() {
        opcode_ = 0;
        payload_.clear();
        handle_.recycle(this);
    }

    uint8_t WebSocketFrame::getOpcode() const {
        return opcode_;
    }

    bool WebSocketFrame::isTextFrame() const {
        return opcode_ == 0x01;
    }

    bool WebSocketFrame::isBinaryFrame() const {
        return opcode_ == 0x02;
    }

    string_view WebSocketFrame::getPayloadAsStringView() const {
        return {reinterpret_cast<const char*>(payload_.data()), payload_.size()};
    }

    tuple<const uint8_t *, size_t> WebSocketFrame::getPayload() const {
        return make_tuple(payload_.data(), payload_.size());
    }

    void WebSocketFrame::setOpcode(const uint8_t op) {
        opcode_ = op;
    }

    void WebSocketFrame::setAsTextFrame() {
        opcode_ = 0x01;
    }

    void WebSocketFrame::setAsBinaryFrame() {
        opcode_ = 0x02;
    }

    void WebSocketFrame::setData(const char *data, const size_t len) {
        payload_.clear();
        payload_.insert(payload_.end(), data, data + len);
    }

    void WebSocketFrame::setData(const string_view data) {
        this->setData(data.data(), data.size());
    }

    void WebSocketFrame::setData(const string &data) {
        this->setData(data.data(), data.size());
    }

    void WebSocketFrame::setData(const vector<uint8_t> &data) {
        payload_.clear();
        payload_.insert_range(payload_.end(), data);
    }

    IMPLEMENT_RECYCLER(WebSocketFrame)

    IMPLEMENT_RECYCLER_GET(WebSocketFrame)
}
