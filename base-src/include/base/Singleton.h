#pragma once

namespace uranus {
    /**
     * Scott Meyers单例模式模板类
     * @tparam T 单例类型
     */
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

        /**
         * 获取单例实列引用
         * @return
         */
        static T &instance() {
            static T _inst;
            return _inst;
        }
    };
}