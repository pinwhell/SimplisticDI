#pragma once

#include <string_view>
#include <unordered_map>
#include <memory>
#include <any>
#include <functional>

namespace simplistic {
    namespace di {
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

            // Primary template for general types
            template<typename T>
            struct is_unique_ptr : std::false_type {};

            // Specialization for unique_ptr
            template<typename T, typename Deleter>
            struct is_unique_ptr<std::unique_ptr<T, Deleter>> : std::true_type {};

            // Helper variable template (C++14 and later)
            template<typename T>
            constexpr bool is_unique_ptr_v = is_unique_ptr<T>::value;

            // Primary template for general types
            template<typename T>
            struct is_shared_ptr : std::false_type {};

            // Specialization for shared_ptr
            template<typename T>
            struct is_shared_ptr<std::shared_ptr<T>> : std::true_type {};

            // Helper variable template (C++14 and later)
            template<typename T>
            constexpr bool is_shared_ptr_v = is_shared_ptr<T>::value;

            // Combined trait to check if a type is any kind of smart pointer
            template<typename T>
            struct is_smart_ptr : std::conditional_t<is_unique_ptr_v<T> || is_shared_ptr_v<T>, std::true_type, std::false_type> {};

            // Helper variable template (C++14 and later)
            template<typename T>
            constexpr bool is_smart_ptr_v = is_smart_ptr<T>::value;
        }

        class IContainer {
        public:
            virtual ~IContainer() = default;

            virtual void InstallAny(uint32_t hash, std::unique_ptr<detail::AnyBase> any) = 0;
            virtual void BindPtr(uint32_t hash, void* binding) = 0;
            virtual bool GetPtr(uint32_t hash, void*& outBinding) const = 0;

            template<typename T>
            inline std::enable_if_t<std::is_pointer_v<T>, IContainer&> Bind(T instance)
            {
                BindPtr(detail::type_hash<std::remove_pointer_t<T>>(), (void*)instance);
                return *this;
            }

            template<typename T>
            inline IContainer& Bind(T* instance)
            {
                return Bind<T*>(instance);
            }

            template<typename T>
            inline std::enable_if_t<std::is_pointer_v<T>, T> Get() const
            {
                void* r = nullptr;
                if (GetPtr(detail::type_hash<std::remove_pointer_t<T>>(), r) == false)
                    return T{};
                return (T)r;
            }

            template<typename T, typename = std::enable_if_t<!std::is_pointer_v<T>, void>>
            inline T* Get() const
            {
                return Get<T*>();
            }

            template<typename T>
            inline T GetO() const
            {
                T* t = Get<T>();
                if (!t)
                    return T{};
                return *t;
            }

            template<
                typename TIface,
                typename TSmartPtr,
                typename TSmartPtrNormal = std::remove_reference_t<std::remove_const_t<TSmartPtr>>,
                typename TElemType = TSmartPtrNormal::element_type
            >
            inline std::enable_if_t<
                detail::is_smart_ptr_v<TSmartPtrNormal>&&
                std::is_rvalue_reference_v<TSmartPtr>, IContainer&
            > Install(TSmartPtr serviceResource)
            {
                if constexpr (detail::is_unique_ptr_v<TSmartPtrNormal>)
                {
                    auto& service = serviceResource;
                    Bind<TIface>(service.get());
                    InstallAny(detail::type_hash<TIface>(), std::make_unique<detail::AnyHolder<TSmartPtrNormal>>(
                        service
                    ));
                }
                else if constexpr (detail::is_shared_ptr_v<TSmartPtrNormal>)
                {
                    Bind<TIface>(serviceResource.get());
                    InstallAny(detail::type_hash<TIface>(), std::make_unique<detail::AnyHolder<TSmartPtrNormal>>(
                        serviceResource
                    ));
                }

                return *this;
            }

            template<typename TIface, typename TSmartPtr,
                typename TSmartPtrNormal = std::remove_reference_t<std::remove_const_t<TSmartPtr>>,
                typename TElemType = TSmartPtrNormal::element_type
            >
            inline std::enable_if_t<
                detail::is_smart_ptr_v<TSmartPtrNormal> &&
                !std::is_rvalue_reference_v<TSmartPtr>, IContainer&> Install(TSmartPtr serviceResource)
            {
                return Install<TIface, TSmartPtr&&>(std::move(serviceResource));
            }

            template<
                typename TObject,
                typename TObjectNormal = std::remove_reference_t<std::remove_const_t<std::remove_pointer_t<TObject>>> /*Smart Pointer Positinoal Place*/
            >
            inline std::enable_if_t<
                !detail::is_smart_ptr_v<TObjectNormal /*Smart Pointer Positinoal Place*/ >,
                IContainer&
            > Install(TObject obj)
            {
                return Install<TObjectNormal>(std::make_unique<TObjectNormal>(obj));
            }
        };

        class Container : public IContainer {
        public:
            inline Container() = default;
            inline Container(Container&& other) noexcept
                : mContainer(std::move(other.mContainer))
                , mAdquisitionsContainer(std::move(other.mAdquisitionsContainer))
            {}

            inline void InstallAny(uint32_t hash, std::unique_ptr<detail::AnyBase> any)
            {
                mAdquisitionsContainer.insert_or_assign(hash, std::move(any));
            }

            inline void BindPtr(uint32_t hash, void* binding)
            {
                mAdquisitionsContainer.erase(hash);
                mContainer.insert_or_assign(hash, binding);
            }

            inline bool GetPtr(uint32_t hash, void*& outBinding) const
            {
                auto it = mContainer.find(hash);
                if (it == mContainer.end())
                    return false;
                outBinding = it->second;
                return true;
            }

            inline Container& operator=(Container&& other) noexcept
            {
                if (this != &other)
                {
                    mContainer = std::move(other.mContainer);
                    mAdquisitionsContainer = std::move(other.mAdquisitionsContainer);
                }
                return *this;
            }

            std::unordered_map<uint32_t, void*> mContainer;
            std::unordered_map<uint32_t, std::unique_ptr<detail::AnyBase>> mAdquisitionsContainer;
        };

        class ContainerScope : public IContainer {
        public:
            inline ContainerScope(IContainer* outterScope)
                : mOutterScope(outterScope)
            {}

            inline ContainerScope(ContainerScope&& other) noexcept
                : mScope(std::move(other.mScope))
            {}

            inline ContainerScope& operator=(ContainerScope&& other) noexcept
            {
                if (this != &other)
                {
                    mScope = std::move(other.mScope);
                }

                return *this;
            }

            inline void InstallAny(uint32_t hash, std::unique_ptr<detail::AnyBase> any) override
            {
                mScope.InstallAny(hash, std::move(any));
            }

            inline void BindPtr(uint32_t hash, void* binding) override
            {
                mScope.BindPtr(hash, binding);
            }

            inline bool GetPtr(uint32_t hash, void*& outBinding) const override
            {
                if (mScope.GetPtr(hash, outBinding))
                    return true;

                return mOutterScope->GetPtr(hash, outBinding);
            }

            Container mScope;
            IContainer* mOutterScope;
        };
    }
}