#pragma once
#include "MoonCore/Core.hpp"
#include "Deleter.hpp"
#include "MoonCore/Async/Atomic.hpp"

namespace MC::Memory
{
    enum class ReferenceMode : u8
    {
        NotThreadSafe = 0,
        ThreadSafe = 1
    };

    constexpr ReferenceMode DEFAULT_REFERENCE_MODE = ReferenceMode::ThreadSafe;

    template<typename T, ReferenceMode Mode, Concept::Deleter<T> D>
    class Reference;

    template<typename T, ReferenceMode Mode, Concept::Deleter<T> D>
    class WeakReference;

    template<typename T, ReferenceMode Mode, Concept::Deleter<T> D>
    class EnableReferenceFromThis;

    namespace Helper
    {
        template<ReferenceMode RefMode>
        class ReferenceStorageBase;

        template<>
        class ReferenceStorageBase<ReferenceMode::ThreadSafe>
        {
        public:
            ReferenceStorageBase() noexcept : m_StrongRefCount(1), m_WeakRefCount(1) {}

            ReferenceStorageBase(const ReferenceStorageBase&) = delete;
            ReferenceStorageBase(ReferenceStorageBase&&) = delete;
            ReferenceStorageBase& operator=(const ReferenceStorageBase&) = delete;
            
        public:
            virtual ~ReferenceStorageBase() = default;
            virtual void DestroyObject() = 0;

        public:
            u64 GetReferenceCount() const noexcept
            {
                return m_StrongRefCount.load(std::memory_order_relaxed);
            }

            void Increment() noexcept
            {
                m_StrongRefCount.fetch_add(1, std::memory_order_relaxed);
            }

            void Decrement() noexcept
            {
                if (m_StrongRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
                {
                    DestroyObject();
                    DecrementWeak();
                }
            }

            void IncrementWeak() noexcept
            {
                m_WeakRefCount.fetch_add(1, std::memory_order_relaxed);
            }

            void DecrementWeak() noexcept
            {
                if (m_WeakRefCount.fetch_sub(1, std::memory_order_acq_rel) == 1)
                    delete this;
            }

            bool IncrementIfAlive() noexcept
            {
                u64 count = m_StrongRefCount.load(std::memory_order_relaxed);
                while (count > 0)
                {
                    if (m_StrongRefCount.compare_exchange_weak(count, count + 1,
                        std::memory_order_relaxed, std::memory_order_relaxed))
                    {
                        return true;
                    }
                }
                return false;
            }

        private:
            Async::AtomicU64 m_StrongRefCount;
            Async::AtomicU64 m_WeakRefCount;
        };

        template<>
        class ReferenceStorageBase<ReferenceMode::NotThreadSafe>
        {
        public:
            ReferenceStorageBase() noexcept : m_StrongRefCount(1), m_WeakRefCount(1) {}

            ReferenceStorageBase(const ReferenceStorageBase&) = delete;
            ReferenceStorageBase(ReferenceStorageBase&&) = delete;
            ReferenceStorageBase& operator=(const ReferenceStorageBase&) = delete;

        public:
            virtual ~ReferenceStorageBase() = default;
            virtual void DestroyObject() = 0;

        public:
            u64 GetReferenceCount() const noexcept
            {
                return m_StrongRefCount;
            }

            void Increment() noexcept
            {
                ++m_StrongRefCount;
            }

            void Decrement() noexcept
            {
                if (--m_StrongRefCount == 0)
                {
                    DestroyObject();
                    DecrementWeak();
                }
            }

            void IncrementWeak() noexcept
            {
                ++m_WeakRefCount;
            }

            void DecrementWeak() noexcept
            {
                if (--m_WeakRefCount == 0)
                    delete this;
            }

            bool IncrementIfAlive() noexcept
            {
                if (m_StrongRefCount == 0) return false;
                ++m_StrongRefCount;
                return true;
            }

        private:
            u64 m_StrongRefCount;
            u64 m_WeakRefCount;
        };

        template<typename T, ReferenceMode Mode, typename D>
        class ReferenceStorage final : public ReferenceStorageBase<Mode>
        {
        public:
            using Pointer = T*;

            explicit ReferenceStorage(Pointer pointer) noexcept : m_Ptr(pointer) {}

            ~ReferenceStorage() noexcept override = default;

            void DestroyObject() noexcept override
            {
                if (m_Ptr)
                {
                    D::Delete(m_Ptr);
                    m_Ptr = nullptr;
                }
            }

        private:
            Pointer m_Ptr;
        };
    }

    template<typename T, ReferenceMode Mode = DEFAULT_REFERENCE_MODE, Concept::Deleter<T> D = DefaultDeleter<T>>
    class Reference
    {
        template<typename U, ReferenceMode UMode, Concept::Deleter<U> UD> friend class Reference;

    public:
        using Pointer = T*;
        using ValueReference = T&;
        
        using StorageType = Helper::ReferenceStorage<T, Mode, D>;
        using StorageBaseType = Helper::ReferenceStorageBase<Mode>;

    public:
        constexpr Reference() noexcept : m_Control(nullptr), m_Ptr(nullptr) {}
        constexpr Reference(NullptrT) noexcept : m_Control(nullptr), m_Ptr(nullptr) {}

        explicit Reference(Pointer pointer) noexcept
        {
            Create(pointer);
        }

        Reference(const Reference& other) noexcept
            : m_Control(other.m_Control), m_Ptr(other.m_Ptr)
        {
            if (m_Control) m_Control->Increment();
        }

        template <typename Y, ReferenceMode OMode, Concept::Deleter<Y> OD>
        Reference(const Reference<Y, OMode, OD>& other, Pointer ptr) noexcept
            : m_Control(other.m_Control), m_Ptr(other.m_Control ? ptr : nullptr)
        {
            if (m_Control) m_Control->Increment();
        }

        template <typename Y, ReferenceMode OMode, Concept::Deleter<Y> OD>
        Reference(const Reference<Y, OMode, OD>& other) noexcept
            : m_Control(other.m_Control), m_Ptr(static_cast<T*>(other.m_Ptr))
        {
            if (m_Control) m_Control->Increment();
        }

        Reference(Reference&& other) noexcept
            : m_Control(other.m_Control), m_Ptr(other.m_Ptr)
        {
            other.m_Control = nullptr;
            other.m_Ptr = nullptr;
        }

        template <typename Y, ReferenceMode OMode, Concept::Deleter<Y> OD>
        Reference(Reference<Y, OMode, OD>&& other, Pointer ptr) noexcept
            : m_Control(other.m_Control), m_Ptr(other.m_Control ? ptr : nullptr)
        {
            other.m_Control = nullptr;
            other.m_Ptr = nullptr;
        }

        template <typename Y, ReferenceMode OMode, Concept::Deleter<Y> OD>
        Reference(Reference<Y, OMode, OD>&& other) noexcept
            : m_Control(other.m_Control), m_Ptr(static_cast<T*>(other.m_Ptr))
        {
            other.m_Control = nullptr;
            other.m_Ptr = nullptr;
        }

        ~Reference() noexcept
        {
            Release();
        }

    private:
        Reference(StorageBaseType* control, Pointer ptr, bool) noexcept
            : m_Control(control), m_Ptr(ptr) {}

    public:
        Reference& operator=(const Reference& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_Control = other.m_Control;
                m_Ptr = other.m_Ptr;
                if (m_Control) m_Control->Increment();
            }
            return *this;
        }

        Reference& operator=(Reference&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_Control = other.m_Control;
                m_Ptr = other.m_Ptr;
                other.m_Control = nullptr;
                other.m_Ptr = nullptr;
            }
            return *this;
        }

    public:
        void Reset() noexcept
        {
            Release();
        }

        template <typename Y>
            requires Concept::ConvertibleTo<Y*, T*>
        void Reset(Y* pointer) noexcept
        {
            Release();
            Create(static_cast<T*>(pointer));
        }

        void Swap(Reference& other) noexcept
        {
            StorageBaseType* tempControl = m_Control;
            T* tempPtr = m_Ptr;
            m_Control = other.m_Control;
            m_Ptr = other.m_Ptr;
            other.m_Control = tempControl;
            other.m_Ptr = tempPtr;
        }
    public:
        template <typename Y, ReferenceMode YMode = Mode, Concept::Deleter<Y> DY = ExtractRebind<D, Y>>
            requires Concept::ConvertibleTo<Y*, T*>
        MC_NODISCARD Reference<Y, YMode, DY> As() const noexcept
        {
            return Reference<Y, YMode, DY>(*this, static_cast<Y*>(m_Ptr));
        }

        template <typename Y>
            requires Concept::ConvertibleTo<Y*, T*>
        MC_NODISCARD Y* GetAs() const noexcept
        {
            return static_cast<Y*>(m_Ptr);
        }

        MC_NODISCARD Pointer Get() const noexcept { return m_Ptr; }
        MC_NODISCARD Pointer operator->() const noexcept { return m_Ptr; }
        MC_NODISCARD ValueReference operator*()  const noexcept { return *m_Ptr; }

        MC_NODISCARD u64 GetReferenceCount() const noexcept
        {
            if (m_Control) return m_Control->GetReferenceCount();
            return 0;
        }

        MC_NODISCARD bool IsValid() const noexcept { return m_Ptr != nullptr; }
        MC_NODISCARD operator bool() const noexcept { return IsValid(); }

        MC_NODISCARD bool operator==(const Reference& other) const noexcept
        {
            return m_Ptr == other.m_Ptr;
        }

        MC_NODISCARD bool operator==(NullptrT) const noexcept
        {
            return m_Ptr == nullptr;
        }

    private:
        void Create(Pointer pointer) noexcept
        {
            if (pointer == nullptr)
            {
                m_Control = nullptr;
                m_Ptr = nullptr;
                return;
            }
            m_Control = new(std::align_val_t{ alignof(StorageType) }, std::nothrow_t()) StorageType(pointer);
            m_Ptr = pointer;

            if constexpr (requires { typename T::ReferenceFromThisType; })
            {
                using ThisBase = typename T::ReferenceFromThisType;
                auto* base = static_cast<ThisBase*>(pointer);
                if (base->m_WeakThis.Expired())
                {
                    base->m_WeakThis.Release();
                    base->m_WeakThis.m_Control = m_Control;
                    base->m_WeakThis.m_Ptr = pointer;
                    m_Control->IncrementWeak();
                }
            }
        }

        void Release() noexcept
        {
            if (m_Control)
            {
                m_Control->Decrement();
                m_Control = nullptr;
                m_Ptr = nullptr;
            }
        }

    private:
        StorageBaseType* m_Control;
        Pointer m_Ptr;

    private:
        template<typename U, ReferenceMode UMode, Concept::Deleter<U> UD>
        friend class WeakReference;
    };

    template<typename T, ReferenceMode Mode = DEFAULT_REFERENCE_MODE, Concept::Deleter<T> D = DefaultDeleter<T>>
    class WeakReference
    {
    public:
        using Pointer = T*;
        using ValueReference = T&;

        using StorageType = Helper::ReferenceStorageBase<Mode>;

    public:
        constexpr WeakReference() noexcept : m_Control(nullptr), m_Ptr(nullptr) {}

        WeakReference(const Reference<T, Mode, D>& sharedRef) noexcept
            : m_Control(sharedRef.m_Control), m_Ptr(sharedRef.m_Ptr)
        {
            if (m_Control) m_Control->IncrementWeak();
        }

        WeakReference(const WeakReference& other) noexcept
            : m_Control(other.m_Control), m_Ptr(other.m_Ptr)
        {
            if (m_Control) m_Control->IncrementWeak();
        }

        WeakReference(WeakReference&& other) noexcept
            : m_Control(other.m_Control), m_Ptr(other.m_Ptr)
        {
            other.m_Control = nullptr;
            other.m_Ptr = nullptr;
        }

        ~WeakReference() noexcept
        {
            Release();
        }

    public:
        WeakReference& operator=(const WeakReference& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_Control = other.m_Control;
                m_Ptr = other.m_Ptr;
                if (m_Control) m_Control->IncrementWeak();
            }
            return *this;
        }

        WeakReference& operator=(WeakReference&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_Control = other.m_Control;
                m_Ptr = other.m_Ptr;
                other.m_Control = nullptr;
                other.m_Ptr = nullptr;
            }
            return *this;
        }

        WeakReference& operator=(const Reference<T, Mode, D>& reference) noexcept
        {
            Release();
            m_Control = reference.m_Control;
            m_Ptr = reference.m_Ptr;
            if (m_Control) m_Control->IncrementWeak();
            return *this;
        }

    public:
        void Reset() noexcept
        {
            Release();
        }

        MC_NODISCARD bool Expired() const noexcept
        {
            return m_Control == nullptr || m_Control->GetReferenceCount() == 0;
        }

        Reference<T, Mode, D> Lock() const noexcept
        {
            if (m_Control && m_Control->IncrementIfAlive())
            {
                return Reference<T, Mode, D>(m_Control, m_Ptr, true);
            }
            return Reference<T, Mode, D>();
        }

    private:
        void Release() noexcept
        {
            if (m_Control)
            {
                m_Control->DecrementWeak();
                m_Control = nullptr;
                m_Ptr = nullptr;
            }
        }

    private:
        StorageType* m_Control;
        Pointer m_Ptr;

    private:
        template<typename U, ReferenceMode UMode, Concept::Deleter<U> UD> friend class Reference;
    };

    template<typename T, ReferenceMode Mode = DEFAULT_REFERENCE_MODE, Concept::Deleter<T> D = DefaultDeleter<T>>
    class EnableReferenceFromThis
    {
    public:
        using ReferenceFromThisType = EnableReferenceFromThis;

    public:
        Reference<T, Mode, D> ReferenceFromThis() const noexcept
        {
            return m_WeakThis.Lock();
        }

        WeakReference<T, Mode, D> WeakFromThis() const noexcept
        {
            return m_WeakThis;
        }

    protected:
        EnableReferenceFromThis() noexcept = default;
        ~EnableReferenceFromThis() noexcept = default;

    private:
        mutable WeakReference<T, Mode, D> m_WeakThis;

        template<typename U, ReferenceMode UMode, Concept::Deleter<U> UD> friend class Reference;
    };

    template<typename T, ReferenceMode Mode = DEFAULT_REFERENCE_MODE, Concept::Deleter<T> D = DefaultDeleter<T>, typename... ArgTypes>
        requires Concept::HasContructor<T, ArgTypes...>
    Reference<T, Mode, D> MakeReference(ArgTypes&&... arguments) noexcept
    {
        return Reference<T, Mode, D>(new(std::nothrow) T(Forward<ArgTypes>(arguments)...));
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> StaticPointerCast(const Reference<T, TMode, DT>& other) noexcept
    {
        auto pointer = static_cast<U*>(other.Get());
        return Reference<U, UMode, DU>(other, pointer);
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> StaticPointerCast(Reference<T, TMode, DT>&& other) noexcept
    {
        auto pointer = static_cast<U*>(other.Get());
        return Reference<U, UMode, DU>(Move(other), pointer);
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> ConstPointerCast(const Reference<T, TMode, DT>& other) noexcept
    {
        auto pointer = const_cast<U*>(other.Get());
        return Reference<U, UMode, DU>(other, pointer);
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> ConstPointerCast(Reference<T, TMode, DT>&& other) noexcept
    {
        auto pointer = const_cast<U*>(other.Get());
        return Reference<U, UMode, DU>(Move(other), pointer);
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> DynamicPointerCast(const Reference<T, TMode, DT>& other) noexcept
    {
        auto pointer = dynamic_cast<U*>(other.Get());
        if (!pointer) return Reference<U, UMode, DU>();
        return Reference<U, UMode, DU>(other, pointer);
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> DynamicPointerCast(Reference<T, TMode, DT>&& other) noexcept
    {
        auto pointer = dynamic_cast<U*>(other.Get());
        if (!pointer) return Reference<U, UMode, DU>();
        return Reference<U, UMode, DU>(Move(other), pointer);
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> ReinterpretPointerCast(const Reference<T, TMode, DT>& other) noexcept
    {
        auto pointer = reinterpret_cast<U*>(other.Get());
        return Reference<U, UMode, DU>(other, pointer);
    }

    template<typename U, ReferenceMode UMode = DEFAULT_REFERENCE_MODE, Concept::Deleter<U> DU = DefaultDeleter<U>,
        typename T, ReferenceMode TMode, Concept::Deleter<T> DT>
    Reference<U, UMode, DU> ReinterpretPointerCast(Reference<T, TMode, DT>&& other) noexcept
    {
        auto pointer = reinterpret_cast<U*>(other.Get());
        return Reference<U, UMode, DU>(Move(other), pointer);
    }
}
