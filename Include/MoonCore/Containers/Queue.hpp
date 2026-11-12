#pragma once
#include "BinaryHeap.hpp"
#include "Deque.hpp"
#include "MoonCore/Callables/Functors.hpp"

namespace MC::Concept
{
    template <class Container>
    concept ContainerEligibleForQueue = IsNothrowDefaultConstructible<Container>
        && requires
    {
        typename Container::ValueType;
        typename Container::RefType;
        typename Container::ConstRefType;
    }
    && requires(Container container, const Container constContainer, const typename Container::ValueType& lValue, typename Container::ValueType&& rValue)
    {
        { container.Front() } -> ConvertibleTo<typename Container::RefType>;
        { constContainer.Front() } -> ConvertibleTo<typename Container::ConstRefType>;
        { container.Back() } -> ConvertibleTo<typename Container::RefType>;
        { constContainer.Back() } -> ConvertibleTo<typename Container::ConstRefType>;
        
        { container.PushBack(lValue) };
        { container.PushBack(rValue) };
        { container.EmplaceBack() };
        { container.PopFront() };
        
        { container.Size() } -> ConvertibleTo<usize>;
        { container.Empty() } -> ConvertibleTo<bool>;
    };

    template <class Container>
    concept ContainerEligibleForPriorityQueue = IsNothrowDefaultConstructible<Container>
        && requires
    {
        typename Container::ValueType;
        typename Container::RefType;
        typename Container::ConstRefType;
        typename Container::Iterator;
    }
    && requires(Container container, const typename Container::ValueType& lValue, typename Container::ValueType&& rValue)
    {
        { container.Front() } -> ConvertibleTo<typename Container::RefType>;
        { const_cast<const Container&>(container).Front() } -> ConvertibleTo<typename Container::ConstRefType>;

        { container.PushBack(lValue) };
        { container.PushBack(rValue) };
        { container.EmplaceBack() };
        { container.PopBack() };

        { container.Begin() } -> ConvertibleTo<typename Container::Iterator>;
        { container.End() } -> ConvertibleTo<typename Container::Iterator>;

        { container.Size() } -> ConvertibleTo<usize>;
        { container.Empty() } -> ConvertibleTo<bool>;
    };
}

namespace MC
{
    template <typename T, class Container = Deque<T>>
        requires Concept::ContainerEligibleForQueue<Container>
    class Queue
    {
    public:
        using ContainerType = Container;
        using ValueType = typename ContainerType::ValueType;
        using RefType = typename ContainerType::RefType;
        using ConstRefType = typename ContainerType::ConstRefType;

    public:
        Queue() noexcept = default;
        explicit Queue(const ContainerType& container) noexcept : m_Container(container) {}
        explicit Queue(ContainerType&& container) noexcept : m_Container(Move(container)) {}
        Queue(const Queue&) noexcept = default;
        Queue(Queue&&) noexcept = default;
        ~Queue() noexcept = default;

    public:
        Queue& operator=(const Queue&) noexcept = default;
        Queue& operator=(Queue&&) noexcept = default;

    public:
        usize Size() const noexcept { return m_Container.Size(); }
        bool Empty() const noexcept { return m_Container.Empty(); }

        RefType Front() noexcept { return m_Container.Front(); }
        ConstRefType Front() const noexcept { return m_Container.Front(); }
        RefType Back() noexcept { return m_Container.Back(); }
        ConstRefType Back() const noexcept { return m_Container.Back(); }

        void Push(const ValueType& value) noexcept { m_Container.PushBack(value); }
        void Push(ValueType&& value) noexcept { m_Container.PushBack(Move(value)); }
        template<typename... ArgTypes> RefType Emplace(ArgTypes&&... args) noexcept { return m_Container.EmplaceBack(Forward<ArgTypes>(args)...); }
        void Pop() noexcept { m_Container.PopFront(); }

    private:
        ContainerType m_Container;
    };

    template <typename T,
        class Compare = Callables::Less<T>,
        class Container = Array<T>,
        class Heap = BinaryHeap<T, Compare, Container>
    >
        requires Concept::HeapType<Heap>
    class PriorityQueue
    {
    public:
        using HeapType = Heap;
        using ContainerType = typename HeapType::ContainerType;
        using ValueType = typename ContainerType::ValueType;
        using RefType = typename ContainerType::RefType;
        using ConstRefType = typename ContainerType::ConstRefType;

    public:
        PriorityQueue() noexcept = default;
        explicit PriorityQueue(const ContainerType& container) noexcept : m_Heap(container) {}
        explicit PriorityQueue(ContainerType&& container) noexcept : m_Heap(Move(container)) {}
        PriorityQueue(const PriorityQueue&) noexcept = default;
        PriorityQueue(PriorityQueue&&) noexcept = default;
        ~PriorityQueue() noexcept = default;

    public:
        PriorityQueue& operator=(const PriorityQueue&) noexcept = default;
        PriorityQueue& operator=(PriorityQueue&&) noexcept = default;

    public:
        usize Size() const noexcept { return m_Heap.Size(); }
        bool Empty() const noexcept { return m_Heap.Empty(); }

        RefType Top() noexcept { return m_Heap.Top(); }
        ConstRefType Top() const noexcept { return m_Heap.Top(); }

        void Push(const ValueType& value) noexcept { m_Heap.Insert(value); }
        void Push(ValueType&& value) noexcept { m_Heap.Insert(Move(value)); }
        template<typename... ArgTypes> RefType Emplace(ArgTypes&&... args) noexcept { return m_Heap.Emplace(Forward<ArgTypes>(args)...); }
        void Pop() noexcept { m_Heap.Pop(); }

    private:
        HeapType m_Heap;
    };
}