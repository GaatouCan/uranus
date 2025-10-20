#pragma once

#include "Common.h"

#include <string>


namespace uranus {
    class AbstractService;
}

class ServiceFactory;

using uranus::AbstractService;


class ServiceHandle final {

public:
    ServiceHandle();

    ServiceHandle(AbstractService *service, ServiceFactory *factory, std::string path);
    ~ServiceHandle();

    DISABLE_COPY(ServiceHandle)

    ServiceHandle(ServiceHandle &&rhs) noexcept;
    ServiceHandle &operator=(ServiceHandle &&rhs) noexcept;

    [[nodiscard]] const std::string &GetPath() const;

    [[nodiscard]] bool IsValid() const;

    AbstractService *operator->() const noexcept;
    AbstractService &operator*() const noexcept;

    [[nodiscard]] AbstractService *Get() const noexcept;

    bool operator==(const ServiceHandle &rhs) const noexcept;
    bool operator==(nullptr_t) const noexcept;

    explicit operator bool() const noexcept;

    void Release();

private:
    AbstractService *service_;
    ServiceFactory *factory_;

    std::string path_;
};
