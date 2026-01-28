#include "Envelope.h"

namespace uranus::actor {
    Envelope::Envelope()
        : tag(0),
          type(0),
          source(0),
          session(0) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, PackageHandle &&pkg)
        : tag(kPackage),
          type(ty),
          source(src),
          session(0),
          variant(std::move(pkg)) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, const int64_t sess, PackageHandle &&pkg)
        : tag(kPackage),
          type(ty),
          source(src),
          session(sess),
          variant(std::move(pkg)) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, const int64_t evt, DataAssetHandle &&data)
        : tag(kDataAsset),
          type(ty),
          source(src),
          event(evt),
          variant(std::move(data)) {
    }

    Envelope::Envelope(const SteadyTimePoint now, const SteadyDuration delta)
        : tag(kTickInfo),
          type(0),
          source(0),
          session(0),
          variant(ActorTickInfo{now, delta}) {
    }

    Envelope::Envelope(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            tag = rhs.tag;
            type = rhs.type;
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
            tag = rhs.tag;
            type = rhs.type;
            source = rhs.source;
            session = rhs.session;

            rhs.type = 0;
            rhs.source = 0;
            rhs.session = 0;

            variant = std::move(rhs.variant);
        }
        return *this;
    }
}