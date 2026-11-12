#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Callables/Functors.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "Details/HeapShared.hpp"

namespace MC::Concept
{
    template <class Container>
    concept ContainerEligibleForBinaryHeap = IsNothrowDefaultConstructible<Container>
    && requires
    {
        typename Container::ValueType;
        typename Container::PtrType;
        typename Container::ConstPtrType;
        typename Container::RefType;
        typename Container::ConstRefType;

        typename Container::Iterator;
        typename Container::ConstIterator;
    }
    && requires(Container container, const Container constContainer, const typename Container::ValueType& lValue, typename Container::ValueType&& rValue)
    {
        { container.Size() } -> ConvertibleTo<usize>;
        { container.Empty() } -> ConvertibleTo<bool>;

        { container.Front() } -> ConvertibleTo<typename Container::RefType>;
        { constContainer.Front() } -> ConvertibleTo<typename Container::ConstRefType>;
        { container.Back() } -> ConvertibleTo<typename Container::RefType>;
        { constContainer.Back() } -> ConvertibleTo<typename Container::ConstRefType>;

        { container.Begin() } -> ConvertibleTo<typename Container::Iterator>;
        { container.CBegin() } -> ConvertibleTo<typename Container::ConstIterator>;
        { container.End() } -> ConvertibleTo<typename Container::Iterator>;
        { container.CEnd() } -> ConvertibleTo<typename Container::ConstIterator>;

        { container.Clear() };
        { container.PushBack(lValue) };
        { container.PushBack(rValue) };
        { container.EmplaceBack() };
        { container.PopBack() };
    };
}

namespace MC
{
    template<typename T, class Compare = Callables::Less<T>, class Container = Array<T>>
        requires Concept::ContainerEligibleForBinaryHeap<Container>
    class BinaryHeap
    {
    public:
        using ContainerType = Container;
        using ValueType = typename ContainerType::ValueType;
        using RefType = typename ContainerType::RefType;
        using ConstRefType = typename ContainerType::ConstRefType;

    public:
        BinaryHeap() noexcept = default;
        explicit BinaryHeap(const ContainerType& container) noexcept : m_Container(container)
        {
            BuildHeap();
        }
        explicit BinaryHeap(ContainerType&& container) noexcept : m_Container(Move(container))
        {
            BuildHeap();
        }
        BinaryHeap(const BinaryHeap&) noexcept = default;
        BinaryHeap(BinaryHeap&&) noexcept = default;
        ~BinaryHeap() noexcept = default;

    public:
        BinaryHeap& operator=(const BinaryHeap&) noexcept = default;
        BinaryHeap& operator=(BinaryHeap&&) noexcept = default;

    public:
        usize Size() const noexcept { return m_Container.Size(); }
        bool Empty() const noexcept { return m_Container.Empty(); }

        RefType Top() noexcept { return m_Container.Front(); }
        ConstRefType Top() const noexcept { return m_Container.Front(); }

    public:
        void Insert(const ValueType& value) noexcept
        {
            m_Container.PushBack(value);
            static_cast<void>(InsertHeap());
        }
        void Insert(ValueType&& value) noexcept
        {
            m_Container.PushBack(Move(value));
            static_cast<void>(InsertHeap());
        }
        template<typename... ArgTypes> 
        RefType Emplace(ArgTypes&&... args) noexcept
        {
            static_cast<void>(m_Container.EmplaceBack(Forward<ArgTypes>(args)...));
            return m_Container[InsertHeap()];
        }
        void Pop() noexcept
        {
            PopHeap();
        }

    private:
        static constexpr usize GetLeftChild(usize n) noexcept { return 2 * n + 1; }
        static constexpr usize GetRightChild(usize n) noexcept { return 2 * n + 2; }
        static constexpr usize GetParent(usize n) noexcept { return n == 0 ? 0 : (n - 1) / 2; }

        void BuildHeap() noexcept
        {
            for (usize i = m_Container.Size() / 2; i > 0; --i)
                PercolateDown(i);
            PercolateDown(0);
        }

        usize InsertHeap() noexcept
        {
            usize hole = m_Container.Size() - 1;
            for (; hole > 0 && m_ComparisonFunctor(m_Container[hole], m_Container[GetParent(hole)]); hole = GetParent(hole))
                Swap(m_Container[hole], m_Container[GetParent(hole)]);
            return hole;
        }

        void PopHeap() noexcept
        {
            Swap(m_Container[0], m_Container[m_Container.Size() - 1]);
            m_Container.PopBack();
            if (!Empty())
                PercolateDown(0);
        }

        void PercolateDown(usize hole) noexcept
        {
            const usize size = m_Container.Size();
            for (usize child = GetLeftChild(hole); child < size; child = GetLeftChild(hole))
            {
                const usize right = child + 1;
                if (right < size && m_ComparisonFunctor(m_Container[right], m_Container[child]))
                    child = right;
                if (!m_ComparisonFunctor(m_Container[child], m_Container[hole])) 
                    break;
                Swap(m_Container[hole], m_Container[child]);
                hole = child;
            }
        }

    private:
        ContainerType m_Container;
        Compare m_ComparisonFunctor;
    };
}