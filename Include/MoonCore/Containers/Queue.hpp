#pragma once
#include "MoonCore/Containers/BinaryHeap.hpp"
#include "MoonCore/Containers/Deque.hpp"
#include "MoonCore/Callables/Functors.hpp"

/**
 * @file Queue.hpp
 * @brief FIFO queue container adapter and priority queue adapter.
 * @ingroup container_adapters
 */

namespace MC::Concept
{
    /**
     * @concept ContainerEligibleForQueue
     * @brief Concept describing container requirements to serve as backing store for `MC::Queue`.
     *
     * Requires `Front()`, `Back()`, `PushBack()`, `EmplaceBack()`, `PopFront()`, `Size()`, and `Empty()`.
     *
     * @tparam Container Backing container type.
     */
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

    /**
     * @concept ContainerEligibleForPriorityQueue
     * @brief Concept describing container requirements to serve as backing store for `MC::PriorityQueue`.
     *
     * @tparam Container Backing container type.
     */
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
    /**
     * @class Queue
     * @brief FIFO (First-In, First-Out) container adapter.
     *
     * `Queue<T, Container>` wraps an underlying sequence container (defaults to `MC::Deque<T>`)
     * to provide strict FIFO queue semantics: elements are inserted at the back and removed from the front.
     *
     * @tparam T The element type stored in the queue.
     * @tparam Container Underlying sequence container. Defaults to `MC::Deque<T>`.
     *
     * @code
     * MC::Queue<int> q;
     * q.Push(1);
     * q.Push(2);
     * int front = q.Front(); // 1
     * q.Pop();               // Removes 1
     * @endcode
     */
    template <typename T, class Container = Deque<T>>
        requires Concept::ContainerEligibleForQueue<Container>
    class Queue
    {
    public:
        using ContainerType = Container;                               ///< Underlying container type.
        using ValueType = typename ContainerType::ValueType;           ///< Stored element type.
        using RefType = typename ContainerType::RefType;               ///< Reference to element.
        using ConstRefType = typename ContainerType::ConstRefType;     ///< Reference to const element.

    public:
        /** @brief Default constructor. Constructs an empty queue. */
        Queue() noexcept = default;

        /**
         * @brief Constructs a queue copying from an existing container.
         * @param container Source container.
         */
        explicit Queue(const ContainerType& container) noexcept : m_Container(container) {}

        /**
         * @brief Constructs a queue moving from an existing container.
         * @param container Source container.
         */
        explicit Queue(ContainerType&& container) noexcept : m_Container(Move(container)) {}

        /** @brief Copy constructor. */
        Queue(const Queue&) noexcept = default;
        /** @brief Move constructor. */
        Queue(Queue&&) noexcept = default;
        /** @brief Destructor. */
        ~Queue() noexcept = default;

    public:
        /** @brief Copy assignment operator. */
        Queue& operator=(const Queue&) noexcept = default;
        /** @brief Move assignment operator. */
        Queue& operator=(Queue&&) noexcept = default;

    public:
        /** @brief Returns number of elements in queue. */
        usize Size() const noexcept { return m_Container.Size(); }
        /** @brief Checks whether queue is empty. */
        bool Empty() const noexcept { return m_Container.Empty(); }

        /** @brief Accesses the first element (oldest element). */
        RefType Front() noexcept { return m_Container.Front(); }
        /** @brief Accesses the first element (const overload). */
        ConstRefType Front() const noexcept { return m_Container.Front(); }
        /** @brief Accesses the last element (newest element). */
        RefType Back() noexcept { return m_Container.Back(); }
        /** @brief Accesses the last element (const overload). */
        ConstRefType Back() const noexcept { return m_Container.Back(); }

        /**
         * @brief Pushes a copy of @p value onto the back of the queue.
         * @param value Element to push.
         */
        void Push(const ValueType& value) noexcept { m_Container.PushBack(value); }

        /**
         * @brief Pushes @p value onto the back of the queue by moving.
         * @param value Element to push.
         */
        void Push(ValueType&& value) noexcept { m_Container.PushBack(Move(value)); }

        /**
         * @brief Constructs an element in-place at the back of the queue.
         * @tparam ArgTypes Constructor argument types.
         * @param args Forwarded constructor arguments.
         * @return Reference to created element.
         */
        template<typename... ArgTypes> RefType Emplace(ArgTypes&&... args) noexcept { return m_Container.EmplaceBack(Forward<ArgTypes>(args)...); }

        /**
         * @brief Removes the front element from the queue.
         */
        void Pop() noexcept { m_Container.PopFront(); }

    private:
        ContainerType m_Container; ///< Underlying container storage.
    };

    /**
     * @class PriorityQueue
     * @brief Strict priority queue container adapter backed by a binary heap.
     *
     * Provides O(1) access to the highest-priority element with O(log N) insertions and removals.
     *
     * @tparam T The element type stored in priority queue.
     * @tparam Compare Strict weak ordering functor. Defaults to `Callables::Less<T>`.
     * @tparam Container Backing random access container. Defaults to `MC::Array<T>`.
     * @tparam Heap Binary heap adapter type. Defaults to `MC::BinaryHeap<T, Compare, Container>`.
     */
    template <typename T,
        class Compare = Callables::Less<T>,
        class Container = Array<T>,
        class Heap = BinaryHeap<T, Compare, Container>
    >
        requires Concept::HeapType<Heap>
    class PriorityQueue
    {
    public:
        using HeapType = Heap;                                         ///< Backing heap implementation.
        using ContainerType = typename HeapType::ContainerType;        ///< Underlying storage container type.
        using ValueType = typename ContainerType::ValueType;           ///< Stored element type.
        using RefType = typename ContainerType::RefType;               ///< Reference to element.
        using ConstRefType = typename ContainerType::ConstRefType;     ///< Reference to const element.

    public:
        /** @brief Default constructor. Constructs an empty priority queue. */
        PriorityQueue() noexcept = default;

        /**
         * @brief Constructs a priority queue from a container by copying and heapifying.
         * @param container Source container.
         */
        explicit PriorityQueue(const ContainerType& container) noexcept : m_Heap(container) {}

        /**
         * @brief Constructs a priority queue from a container by moving and heapifying.
         * @param container Source container.
         */
        explicit PriorityQueue(ContainerType&& container) noexcept : m_Heap(Move(container)) {}

        /** @brief Copy constructor. */
        PriorityQueue(const PriorityQueue&) noexcept = default;
        /** @brief Move constructor. */
        PriorityQueue(PriorityQueue&&) noexcept = default;
        /** @brief Destructor. */
        ~PriorityQueue() noexcept = default;

    public:
        /** @brief Copy assignment operator. */
        PriorityQueue& operator=(const PriorityQueue&) noexcept = default;
        /** @brief Move assignment operator. */
        PriorityQueue& operator=(PriorityQueue&&) noexcept = default;

    public:
        /** @brief Returns number of elements. */
        usize Size() const noexcept { return m_Heap.Size(); }
        /** @brief Checks whether priority queue has no elements. */
        bool Empty() const noexcept { return m_Heap.Empty(); }

        /**
         * @brief Accesses the highest-priority element.
         * @return Mutable reference to top element.
         */
        RefType Top() noexcept { return m_Heap.Top(); }

        /**
         * @brief Accesses the highest-priority element (const overload).
         * @return Const reference to top element.
         */
        ConstRefType Top() const noexcept { return m_Heap.Top(); }

        /**
         * @brief Pushes a copy of @p value into the priority queue.
         * @param value Element to push.
         */
        void Push(const ValueType& value) noexcept { m_Heap.Insert(value); }

        /**
         * @brief Pushes @p value into the priority queue by moving.
         * @param value Element to push.
         */
        void Push(ValueType&& value) noexcept { m_Heap.Insert(Move(value)); }

        /**
         * @brief Constructs an element in-place in the priority queue.
         * @tparam ArgTypes Constructor argument types.
         * @param args Forwarded constructor arguments.
         * @return Reference to created element.
         */
        template<typename... ArgTypes> RefType Emplace(ArgTypes&&... args) noexcept { return m_Heap.Emplace(Forward<ArgTypes>(args)...); }

        /**
         * @brief Removes the highest-priority element.
         */
        void Pop() noexcept { m_Heap.Pop(); }

    private:
        HeapType m_Heap; ///< Underlying binary heap instance.
    };
}
