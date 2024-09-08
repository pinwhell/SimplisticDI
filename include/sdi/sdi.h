#pragma once

#include <string_view>
#include <unordered_map>
#include <memory>
#include <any>
#include <functional>

namespace sdi {

    namespace detail {
        // FNV-1a hash function at compile-time
        consteval uint32_t fnv1a_32(const char* s, std::size_t count) {
            uint32_t hash = 0x811c9dc5u;
            for (std::size_t i = 0; i < count; ++i) {
                hash ^= static_cast<uint32_t>(s[i]);
                hash *= 0x01000193u;
            }
            return hash;
        }

        template <typename T>
        consteval std::string_view type_name() {
#if defined(__clang__) || defined(__GNUC__)
            return __PRETTY_FUNCTION__; // GCC or Clang
#elif defined(_MSC_VER)
            return __FUNCSIG__; // MSVC
#else
            return "unknown";
#endif
        }

        template <typename T>
        consteval uint32_t type_hash() {
            constexpr auto name = type_name<T>();
            return fnv1a_32(name.data(), name.size());
        }

        class AnyBase {
        public:
            virtual ~AnyBase() = default;
        };

        template <typename T>
        class AnyHolder : public AnyBase {
        public:
            explicit AnyHolder(T& value) : value_(std::move(value)) {}
        private:
            T value_;
        };
    }

    class Container {
    public:
        Container() = default;
        Container(Container&& other) noexcept
            : mContainer(std::move(other.mContainer))
            , mOwnershipsContainer(std::move(other.mOwnershipsContainer))
        {}

        template<typename T>
        Container& Bind(T instance)
        {
            mContainer.insert_or_assign(detail::type_hash<T>(), instance);
            return *this;
        }

        template<typename T>
        T Get() const
        {
            if (mContainer.find(detail::type_hash<T>()) == mContainer.end())
                return {};

            return std::any_cast<T>(mContainer.at(detail::type_hash<T>()));
        }

        template<typename T>
        Container& BindIface(T* instance)
        {
            mOwnershipsContainer.erase(detail::type_hash<T>());
            return Bind<T*>(instance);
        }

        template<typename T>
        T* GetIface() const
        {
            return Get<T*>();
        }

        template<typename T>
        Container& Install(std::unique_ptr<T>&& _service)
        {
            std::unique_ptr<T>& service = _service;
            BindIface<T>(service.get());
            mOwnershipsContainer[detail::type_hash<T>()] = 
                std::make_unique<detail::AnyHolder<std::unique_ptr<T>>>(
                    service
                );
            return *this;
        }

        template<typename T>
        Container& SharedInstall(std::shared_ptr<T> service)
        {
            BindIface<T>(service.get());
            mOwnershipsContainer[detail::type_hash<T>()] = 
                std::make_unique<detail::AnyHolder<std::shared_ptr<T>>>(
                    service
                );
            return *this;
        }

        Container& operator=(Container&& other) noexcept
        {
            if (this != &other)
            {
                mContainer = std::move(other.mContainer);
                mOwnershipsContainer = std::move(other.mOwnershipsContainer);
            }
        }

        std::unordered_map<uint32_t, std::any> mContainer;
        std::unordered_map<uint32_t, std::unique_ptr<detail::AnyBase>> mOwnershipsContainer;
    };
}