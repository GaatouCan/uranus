#include "HttpRequest.h"
#include "HttpUtils.h"

#include <nlohmann/json.hpp>


namespace uranus::http {
    HttpRequest::HttpRequest(const HttpRequestRecyclerHandle &handle)
        : handle_(handle),
          method_(-1),
          remotePort_(0) {
    }

    HttpRequest::~HttpRequest() {
    }

    int HttpRequest::getMethod() const {
        return method_;
    }

    const std::string &HttpRequest::getUrl() const {
        return path_;
    }

    bool HttpRequest::hasHeader(const std::string &key) const {
        if (IsProhibitedHeaderName(key))
            return false;
        return headers_.contains(key);
    }

    auto HttpRequest::getHeaders(const std::string &key) const -> std::pair<Headers::const_iterator, Headers::const_iterator> {
        if (IsProhibitedHeaderName(key))
            return {};
        return headers_.equal_range(key);
    }

    std::string HttpRequest::getHeader(const std::string &key, const size_t id, const std::string &def) const {
        auto [beg, end] = getHeaders(key);
        std::advance(beg, id);
        if (beg != end) {
            return beg->second;
        }
        return def;
    }

    size_t HttpRequest::getHeaderValueAsULongLong(const std::string &key, const size_t id, const size_t def) const {
        const auto val = getHeader(key, id);
        if (val.empty()) {
            return def;
        }

        if (detail::IsNumeric(val)) {
            return std::strtoull(val.data(), nullptr, 10);
        }

        return def;
    }

    size_t HttpRequest::getHeaderValueCount(const std::string &key) const {
        auto [beg, end] = getHeaders(key);
        return std::distance(beg, end);
    }

    bool HttpRequest::hasParam(const std::string &key) const {
        return params_.contains(key);
    }

    std::string HttpRequest::getParam(const std::string &key, const size_t id) const {
        auto [beg, end] = params_.equal_range(key);
        std::advance(beg, id);
        return beg != end ? beg->second : std::string();
    }

    size_t HttpRequest::getParamValueCount(const std::string &key) const {
        auto [beg, end] = params_.equal_range(key);
        return std::distance(beg, end);
    }

    bool HttpRequest::isMultipartFormData() const {
        const auto type = getHeader("Content-Type");
        return !type.rfind("multipart/form-data", 0);
    }

    std::string HttpRequest::getBearerTokenAuth() const {
        if (hasHeader("Authorization")) {
            static constexpr auto len = detail::StringLength("Bearer ");
            return getHeader("Authorization").substr(len);
        }
        return {};
    }

    bool HttpRequest::isChunkedTransferEncoding() const {
        return detail::case_ignore::equal(getHeader("Transfer-Encoding", 0), "chunked");
    }

    IMPLEMENT_RECYCLER_GET(HttpRequest)

    void HttpRequest::recycle() {
        method_ = -1;

        target_.clear();
        version_.clear();
        path_.clear();

        params_.clear();
        headers_.clear();
        tailers_.clear();

        body_.clear();

        remoteAddr_.clear();
        remotePort_ = 0;

        pathParams_.clear();
        ranges_.clear();

        handle_.recycle(this);
    }

    std::string HttpRequest::toString() const {
        nlohmann::json json;

        json["method"] = GetMethodName(method_);
        json["uri"] = path_;
        json["params"] = nlohmann::json::array();

        for (const auto &[k, v] : params_) {
            nlohmann::json param;
            param[k] = v;
            json["params"].push_back(param);
        }

        json["headers"] = nlohmann::json::array();

        for (const auto &[k, v] : headers_) {
            nlohmann::json header;
            header[k] = v;
            json["headers"].push_back(header);
        }

        return json.dump();
    }

    IMPLEMENT_RECYCLER(HttpRequest)
}
