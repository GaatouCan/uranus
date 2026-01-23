#include "Envelope.h"

namespace uranus::actor {
    Envelope::Envelope()
        : type(0),
          source(0),
          session(0),
          package(nullptr) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, PackageHandle &&pkg)
        : type(ty),
          source(src),
          session(0),
          package(std::move(pkg)) {
    }

    Envelope::Envelope(const int32_t ty, const int64_t src, const int64_t sess, PackageHandle &&pkg)
        : type(ty),
          source(src),
          session(sess),
          package(std::move(pkg)) {
    }

    Envelope::Envelope(Envelope &&rhs) noexcept {
        if (this != &rhs) {
            type = rhs.type;
            source = rhs.source;
            session = rhs.session;

            rhs.type = 0;
            rhs.source = 0;
            rhs.session = 0;

            package = std::move(rhs.package);
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

            package = std::move(rhs.package);
        }
        return *this;
    }
}