#include "Envelope.h"

namespace uranus::actor {
    Envelope::Envelope()
        : type(0),
          flag(0),
          source(0),
          session(0) {
    }

    Envelope::Envelope(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            type = rhs.type;
            flag = rhs.flag;
            source = rhs.source;
            session = rhs.session;

            rhs.type = 0;
            rhs.source = 0;
            rhs.session = 0;

            variant = std::move(rhs.variant);
        }
    }

    Envelope &Envelope::operator=(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            type = rhs.type;
            flag = rhs.flag;
            source = rhs.source;
            session = rhs.session;

            rhs.type = 0;
            rhs.source = 0;
            rhs.session = 0;

            variant = std::move(rhs.variant);
        }
        return *this;
    }

    Envelope Envelope::makePackage(const int flag, const int64_t src, PackageHandle &&pkg) {
        Envelope evl;

        evl.type = kPackage;
        evl.flag = flag;
        evl.source = src;
        evl.variant = std::move(pkg);

        return evl;
    }

    Envelope Envelope::makeRequest(const int flag, const int64_t src, const int64_t sess, PackageHandle &&req) {
        Envelope evl;

        evl.type = kRequest;
        evl.flag = flag;
        evl.source = src;
        evl.session = sess;

        evl.variant = std::move(req);

        return evl;
    }

    Envelope Envelope::makeResponse(const int flag, const int64_t src, const int64_t sess, PackageHandle &&res) {
        Envelope evl;

        evl.type = kResponse;
        evl.flag = flag;
        evl.source = src;
        evl.session = sess;

        evl.variant = std::move(res);

        return evl;
    }

    Envelope Envelope::makeDataAsset(const int64_t evt, DataAssetHandle &&data) {
        Envelope evl;

        evl.type = kDataAsset;
        evl.event = evt;

        evl.variant = std::move(data);

        return evl;
    }

    Envelope Envelope::makeTickInfo(const SteadyTimePoint now, const SteadyDuration delta) {
        Envelope evl;

        evl.type = kTickInfo;
        evl.variant = ActorTickInfo{now, delta};

        return evl;
    }

    Envelope Envelope::makeCallback(const ActorCallback &cb) {
        Envelope evl;

        evl.type = kCallback;
        evl.variant = cb;

        return evl;
    }
}
