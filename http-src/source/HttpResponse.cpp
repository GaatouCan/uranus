#include "HttpResponse.h"
#include "HttpUtils.h"

namespace uranus::http {
    HttpResponse::HttpResponse(const HttpResponseRecyclerHandle &handle)
        : handle_(handle),
          status_(-1),
          contentLength_(0) {
    }

    HttpResponse::~HttpResponse() {
    }

    void HttpResponse::recycle() {
        status_ = -1;

        headers_.clear();
        trailers_.clear();

        body_.clear();
        location_.clear();

        contentLength_ = 0;

        handle_.recycle(this);
    }

    int HttpResponse::getStatus() const {
        return status_;
    }

    void HttpResponse::setStatus(const StatusCode code) {
        status_ = static_cast<int>(code);
    }

    bool HttpResponse::hasHeader(const std::string &key) const {
        return headers_.contains(key);
    }

    auto HttpResponse::getHeaders(const std::string &key) const -> std::pair<Headers::const_iterator, Headers::const_iterator> {
        if (IsProhibitedHeaderName(key))
            return {};
        return headers_.equal_range(key);
    }

    std::string HttpResponse::getHeader(const std::string &key, const size_t id, const std::string &def) const {
        auto [beg, end] = getHeaders(key);
        std::advance(beg, id);
        if (beg != end) {
            return beg->second;
        }
        return def;
    }

    void HttpResponse::setHeader(const std::string &key, const std::string &value) {
        if (field::IsFieldName(key) && field::IsFieldValue(value)) {
            headers_.emplace(key, value);
        }
    }

    void HttpResponse::setContent(const std::string &data, const std::string &type) {
        body_ = data;

        const auto [beg, end] = headers_.equal_range(std::string_view("Content-Type"));
        headers_.erase(beg, end);
        headers_.emplace("Content-Type", type);
    }

    IMPLEMENT_RECYCLER_GET(HttpResponse)

    IMPLEMENT_RECYCLER(HttpResponse)
}
