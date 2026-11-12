#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/TypeTraits.hpp"
#include "MoonCore/Concept.hpp"

namespace MC::Concept
{
    template <class T>
    concept WeaklyIncrementable = Moveable<T>
    && requires(T it)
    {
        typename T::DiffType;
        typename RemoveCVReference<typename T::DiffType>;
        requires SignedIntegral<typename T::DiffType>;
        { ++it } -> SameAs<T&>;
        it++;
    };

    template<class T>
    concept InputIteratorType = WeaklyIncrementable<T>
        && requires
    {
        typename T::ValueType;
        typename T::PtrType;
        typename T::RefType;
        typename T::Category;
    }
    && requires(T it)
    {
        { *it } -> CanReference;
        { it.operator->() } -> SameAs<typename T::PtrType>;
        { it == it } -> ConvertibleTo<bool>;
        { it != it } -> ConvertibleTo<bool>;
        { it++ } -> SameAs<T>;
    };

    template<class T, class ValueType>
    concept OutputIteratorType = WeaklyIncrementable<T>
        && requires(T it, ValueType val)
    {
        { *it = val } -> CanReference;
        { *it = Move(val) };
        *it++;
    };

    template<class T>
    concept ForwardIteratorType = InputIteratorType<T>
        && WeaklyIncrementable<T>
        && DefaultInitiazable<T>
        && requires(T it)
    {
        { it++ } -> SameAs<T>;
    };

    template<class T>
    concept BidirectionalIteratorType = ForwardIteratorType<T>
        && requires(T it)
    {
        { --it } -> SameAs<T&>;
        { it-- } -> SameAs<T>;
    };

    template<class T>
    concept RandomAccessIteratorType = BidirectionalIteratorType<T>
        && requires(T it, typename T::DiffType n)
    {
        { it + n } -> SameAs<T>;
        { it - n } -> SameAs<T>;
        { it += n } -> SameAs<T&>;
        { it -= n } -> SameAs<T&>;
        { it - it } -> SameAs<typename T::DiffType>;
        { it[n] } -> SameAs<typename T::RefType>;
        { it < it } -> ConvertibleTo<bool>;
        { it > it } -> ConvertibleTo<bool>;
    };

    template<class T>
    concept ContiguousIteratorType = RandomAccessIteratorType<T>
        && requires(T it)
    {
        { it.operator->() } -> SameAs<typename T::PtrType>;
        requires IsPointer<typename T::PtrType>;
    };
}

namespace MC::Iterator
{
    namespace Tags
    {
        struct Input {};
        struct Output {};
        struct Forward : Input {};
        struct Bidirectional : Forward {};
        struct RandomAccess : Bidirectional {};
        struct Contiguous : RandomAccess {};
    }

    template <class Container>
    class BackInsertIterator
    {
    public:
        using ValueType = void;
        using DiffType  = iptr;
        using PtrType   = void;
        using RefType   = void;
        using Category  = Tags::Bidirectional;

        using ContainerType = Container;

    public:
        explicit constexpr BackInsertIterator(Container& container) noexcept
            : m_Container(&container)
        {}

        constexpr BackInsertIterator& operator=(const typename ContainerType::ValueType& value) noexcept
        {
            m_Container->PushBack(value);
            return *this;
        }

        constexpr BackInsertIterator& operator=(typename ContainerType::ValueType&& value) noexcept
        {
            m_Container->PushBack(Move(value));
            return *this;
        }

        MC_NODISCARD BackInsertIterator& operator*() noexcept { return *this; }
        constexpr BackInsertIterator& operator++() noexcept { return *this; }
        constexpr BackInsertIterator operator++(int) noexcept { return *this; }

    private:
        ContainerType* m_Container;
    };

    template<Concept::BidirectionalIteratorType Iterator>
    class ReverseIterator
    {
    public:
        using ValueType = typename Iterator::ValueType;
        using DiffType  = typename Iterator::DiffType;
        using PtrType   = typename Iterator::PtrType;
        using RefType   = typename Iterator::RefType;
        using Category  = typename Iterator::Category;

    public:
        constexpr ReverseIterator() noexcept : m_Ptr(nullptr) {}
        constexpr ReverseIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

    public:
        constexpr ReverseIterator& operator+=(usize index) noexcept { m_Ptr -= index; return *this; }
        constexpr ReverseIterator& operator-=(usize index) noexcept { m_Ptr += index; return *this; }

        constexpr ReverseIterator operator+(usize index) const noexcept { return ReverseIterator(m_Ptr - index); }
        constexpr ReverseIterator operator-(const usize& index) const noexcept { return ReverseIterator(m_Ptr + index); }

        constexpr ReverseIterator& operator++() noexcept { --m_Ptr; return *this; }
        constexpr ReverseIterator operator++(int) noexcept { ReverseIterator temp = *this; ++(*this); return temp; }

        constexpr ReverseIterator& operator--() noexcept { ++m_Ptr; return (*this); }
        constexpr ReverseIterator operator--(int) noexcept { ReverseIterator temp = *this; --(*this); return temp; }

        constexpr DiffType operator-(const ReverseIterator& it) const noexcept { return m_Ptr - it.m_Ptr; }

        constexpr bool operator==(const ReverseIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
        constexpr bool operator!=(const ReverseIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }
        constexpr bool operator<(const ReverseIterator& it) const noexcept { return m_Ptr < it.m_Ptr; }
        constexpr bool operator>(const ReverseIterator& it) const noexcept { return m_Ptr > it.m_Ptr; }

        constexpr RefType operator[](usize index) const noexcept { return *(m_Ptr - index); }
        constexpr PtrType operator->() const noexcept { return m_Ptr; }
        constexpr RefType operator*() const noexcept { return *m_Ptr; }

    private:
        PtrType m_Ptr;
    };
}