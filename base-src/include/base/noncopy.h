#pragma once

#define DISABLE_COPY(clazz) \
clazz(const clazz&) = delete; \
clazz &operator=(const clazz&) = delete;

#define DISABLE_MOVE(clazz) \
clazz(clazz &&) noexcept = delete; \
clazz &operator=(clazz &&) noexcept = delete;

#define DISABLE_COPY_MOVE(clazz) \
DISABLE_COPY(clazz) \
DISABLE_MOVE(clazz)