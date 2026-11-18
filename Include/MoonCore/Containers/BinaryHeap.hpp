#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Callables/Functors.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Containers/Details/HeapShared.hpp"

/**
 * @file BinaryHeap.hpp
 * @brief Binary heap priority queue container.
 * @ingroup container_adapters
 */

namespace MC::Concept
{
    /**
     * @concept ContainerEligibleForBinaryHeap
     * @brief Concept describing container requirements to serve as the backing store for `MC::BinaryHeap`.
     *
     * Requires value, pointer, reference, and iterator types, plus size queries (`Size()`, `Empty()`),
     * element access (`Front()`, `Back()`), sequential iterators (`Begin()`, `End()`), and modification
     * operations (`PushBack()`, `EmplaceBack()`, `PopBack()`, `Clear()`).
     *
     * @tparam Container Backing container type.
     */
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
    /**
     * @class BinaryHeap
     * @brief High-performance binary heap container (priority queue).
     *
     * `BinaryHeap<T, Compare, Container>` provides logarithmic time insertion (O(log N))
     * and deletion (O(log N)) of the highest (or lowest, depending on @p Compare) priority element,
     * with constant time (O(1)) retrieval of the top element.
     *
     * By default, uses `Callables::Less<T>` producing a max-heap (largest element at the root),
     * backed by an `MC::Array<T>`.
     *
     * @tparam T The element type stored in the heap.
     * @tparam Compare Strict weak ordering functor. Defaults to `Callables::Less<T>`.
     * @tparam Container Underlying random-access container. Defaults to `MC::Array<T>`.
     *
     * @code
     * MC::BinaryHeap<int> heap;
     * heap.Insert(30);
     * heap.Insert(10);
     * heap.Insert(50);
     *
     * int top = heap.Top(); // 50
     * heap.Pop();          // Removes 50; new top is 30
     * @endcode
     */
    template<typename T, class Compare = Callables::Less<T>, class Container = Array<T>>
        requires Concept::ContainerEligibleForBinaryHeap<Container>
    class BinaryHeap
    {
    public:
        using ContainerType = Container;                               ///< Underlying container type.
        using ValueType = typename ContainerType::ValueType;           ///< Stored element type.
        using RefType = typename ContainerType::RefType;               ///< Reference to element.
        using ConstRefType = typename ContainerType::ConstRefType;     ///< Reference to const element.

    public:
        /** @brief Default constructor. Constructs an empty heap. */
        BinaryHeap() noexcept = default;

        /**
         * @brief Constructs a heap from an existing container by copying and heapifying (O(N)).
         * @param container Source container to copy from.
         */
        explicit BinaryHeap(const ContainerType& container) noexcept : m_Container(container)
        {
            BuildHeap();
        }

        /**
         * @brief Constructs a heap from an existing container by moving and heapifying (O(N)).
         * @param container Source container to move from.
         */
        explicit BinaryHeap(ContainerType&& container) noexcept : m_Container(Move(container))
        {
            BuildHeap();
        }

        /** @brief Copy constructor. */
        BinaryHeap(const BinaryHeap&) noexcept = default;
        /** @brief Move constructor. */
        BinaryHeap(BinaryHeap&&) noexcept = default;
        /** @brief Destructor. */
        ~BinaryHeap() noexcept = default;

    public:
        /** @brief Copy assignment operator. */
        BinaryHeap& operator=(const BinaryHeap&) noexcept = default;
        /** @brief Move assignment operator. */
        BinaryHeap& operator=(BinaryHeap&&) noexcept = default;

    public:
        /** @brief Returns number of elements in heap. */
        usize Size() const noexcept { return m_Container.Size(); }
        /** @brief Checks if heap has no elements. */
        bool Empty() const noexcept { return m_Container.Empty(); }

        /**
         * @brief Accesses the top (highest-priority) element.
         * @return Mutable reference to top element.
         * @note O(1) time complexity.
         */
        RefType Top() noexcept { return m_Container.Front(); }

        /**
         * @brief Accesses the top (highest-priority) element (const overload).
         * @return Const reference to top element.
         * @note O(1) time complexity.
         */
        ConstRefType Top() const noexcept { return m_Container.Front(); }

    public:
        /**
         * @brief Inserts a copy of @p value into the heap and restores heap order.
         * @param value Element to insert.
         * @note O(log N) time complexity.
         */
        void Insert(const ValueType& value) noexcept
        {
            m_Container.PushBack(value);
            static_cast<void>(InsertHeap());
        }

        /**
         * @brief Inserts @p value by moving into the heap and restores heap order.
         * @param value Element to insert.
         * @note O(log N) time complexity.
         */
        void Insert(ValueType&& value) noexcept
        {
            m_Container.PushBack(Move(value));
            static_cast<void>(InsertHeap());
        }

        /**
         * @brief Constructs an element in-place and restores heap order.
         * @tparam ArgTypes Argument types for element constructor.
         * @param args Forwarded arguments.
         * @return Reference to the inserted element.
         * @note O(log N) time complexity.
         */
        template<typename... ArgTypes> 
        RefType Emplace(ArgTypes&&... args) noexcept
        {
            static_cast<void>(m_Container.EmplaceBack(Forward<ArgTypes>(args)...));
            return m_Container[InsertHeap()];
        }

        /**
         * @brief Removes the top (highest-priority) element from the heap and restores heap order.
         * @note O(log N) time complexity.
         */
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
        ContainerType m_Container;     ///< Underlying sequence container.
        Compare m_ComparisonFunctor;   ///< Strict weak ordering comparison functor.
    };
}
