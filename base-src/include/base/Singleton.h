#pragma once

namespace uranus {

    template<class T>
    class Singleton {

    protected:
        Singleton() = default;
        virtual ~Singleton() = default;

    public:
        Singleton(Singleton const&) = delete;
        Singleton& operator=(Singleton const&) = delete;

        Singleton(Singleton&&) noexcept = delete;
        Singleton& operator=(Singleton&&) noexcept = delete;

        static T &instance() {
            static T _inst;
            return _inst;
        }
    };
}