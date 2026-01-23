#include "Envelope.h"

namespace uranus::actor {
    Envelope::Envelope()
        : type(0),
          source(0),
          session(0) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, PackageHandle &&pkg)
        : type(ty),
          source(src),
          session(0),
          variant(std::move(pkg)) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, const int64_t sess, PackageHandle &&pkg)
        : type(ty),
          source(src),
          session(sess),
          variant(std::move(pkg)) {
    }

    Envelope::Envelope(int32_t ty, int64_t src, int64_t evt, DataAssetHandle &&data)
        : type(ty),
          source(src),
          event(evt),
          variant(std::move(data)) {
    }

    Envelope::Envelope(Envelope &&rhs) noexcept {
        if (this != &rhs) {
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