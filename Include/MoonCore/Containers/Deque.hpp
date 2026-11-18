#pragma once
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/Iterator.hpp"

/**
 * @file Deque.hpp
 * @brief Segmented double-ended queue container.
 * @ingroup seq_containers
 */

namespace MC
{
    /**
     * @class DequeIterator
     * @brief Random-access iterator over chunked blocks of `MC::Deque`.
     *
     * Handles linear indexing arithmetic transparently across discontinuous memory blocks.
     *
     * @tparam Container Owning `MC::Deque` container type.
     * @tparam IsConst Whether the iterator grants const access.
     */
    template<typename Container, bool IsConst = false>
    class DequeIterator
    {
    private:
        static constexpr usize ELEMCNT_IN_BLOCK_COUNT = Container::ELEMCNT_IN_BLOCK_COUNT;
        friend class DequeIterator<Container, !IsConst>;

    public:
        using ValueType = typename Container::ValueType;               ///< Element type.
        using DiffType = iptr;                                         ///< Iterator distance difference type.
        using PtrType = Conditional<IsConst, const ValueType*, ValueType*>; ///< Pointer to element.
        using RefType = Conditional<IsConst, const ValueType&, ValueType&>; ///< Reference to element.
        using Category = Iterator::Tags::RandomAccess;                 ///< Random access iterator tag.
        using BlockType = typename Container::BlockType;               ///< Internal block type.

    public:
        /** @brief Default constructs an invalid iterator. */
        constexpr DequeIterator() noexcept
            : m_Map(nullptr), m_CurrentBlock(0), m_CurrentElement(0)
        {}

        /**
         * @brief Constructs an iterator targeting a specific block and element index.
         * @param map Array of block pointers.
         * @param block Target block index.
         * @param element Target element index within the block.
         */
        constexpr DequeIterator(BlockType** map, usize block, usize element) noexcept
            : m_Map(map), m_CurrentBlock(block), m_CurrentElement(element)
        {}

        /**
         * @brief Conversion constructor from non-const to const iterator.
         * @tparam OtherConst Boolean flag for source iterator constness.
         * @param other Source non-const iterator.
         */
        template<bool OtherConst> requires (IsConst && !OtherConst)
            constexpr DequeIterator(const DequeIterator<Container, OtherConst>& other) noexcept
            : m_Map(other.m_Map), m_CurrentBlock(other.m_CurrentBlock), m_CurrentElement(other.m_CurrentElement)
        {}

    public:
        /** @brief Advances iterator forward by @p offset positions. */
        constexpr DequeIterator& operator+=(DiffType offset) noexcept
        {
            constexpr DiffType N = static_cast<DiffType>(ELEMCNT_IN_BLOCK_COUNT);
            DiffType absoluteIndex = static_cast<DiffType>(m_CurrentElement) + offset;
            DiffType blockDelta = absoluteIndex / N;
            DiffType elementInBlock = absoluteIndex % N;

            if (elementInBlock < 0)
            {
                elementInBlock += N;
                blockDelta -= 1;
            }

            m_CurrentElement = static_cast<usize>(elementInBlock);
            m_CurrentBlock = static_cast<usize>(static_cast<DiffType>(m_CurrentBlock) + blockDelta);
            return *this;
        }

        /** @brief Decrements iterator backward by @p offset positions. */
        constexpr DequeIterator& operator-=(DiffType offset) noexcept { return *this += -offset; }

        /** @brief Returns a copy of the iterator advanced by @p offset. */
        constexpr DequeIterator operator+(DiffType offset) const noexcept { DequeIterator r = *this; r += offset; return r; }
        /** @brief Returns a copy of the iterator decremented by @p offset. */
        constexpr DequeIterator operator-(DiffType offset) const noexcept { DequeIterator r = *this; r -= offset; return r; }
        /** @brief Friend addition operator. */
        friend constexpr DequeIterator operator+(DiffType offset, const DequeIterator& it) noexcept { return it + offset; }

        /** @brief Pre-increment operator. */
        constexpr DequeIterator& operator++() noexcept
        {
            if (++m_CurrentElement == ELEMCNT_IN_BLOCK_COUNT)
            {
                m_CurrentElement = 0;
                ++m_CurrentBlock;
            }
            return *this;
        }

        /** @brief Post-increment operator. */
        constexpr DequeIterator operator++(int) noexcept { DequeIterator t = *this; ++(*this); return t; }

        /** @brief Pre-decrement operator. */
        constexpr DequeIterator& operator--() noexcept
        {
            if (m_CurrentElement == 0)
            {
                m_CurrentElement = ELEMCNT_IN_BLOCK_COUNT - 1;
                --m_CurrentBlock;
            }
            else
                --m_CurrentElement;
            return *this;
        }

        /** @brief Post-decrement operator. */
        constexpr DequeIterator operator--(int) noexcept { DequeIterator t = *this; --(*this); return t; }

        /** @brief Computes distance between two iterators. */
        constexpr DiffType operator-(const DequeIterator& other) const noexcept
        {
            DiffType blockDiff = static_cast<DiffType>(m_CurrentBlock) - static_cast<DiffType>(other.m_CurrentBlock);
            DiffType elementDiff = static_cast<DiffType>(m_CurrentElement) - static_cast<DiffType>(other.m_CurrentElement);
            return blockDiff * static_cast<DiffType>(ELEMCNT_IN_BLOCK_COUNT) + elementDiff;
        }

        /** @brief Equality comparison. */
        constexpr bool operator==(const DequeIterator& other) const noexcept
        {
            return m_CurrentBlock == other.m_CurrentBlock && m_CurrentElement == other.m_CurrentElement;
        }

        /** @brief Ordering comparison. */
        constexpr bool operator<(const DequeIterator& other) const noexcept
        {
            if (m_CurrentBlock != other.m_CurrentBlock)
                return m_CurrentBlock < other.m_CurrentBlock;
            return m_CurrentElement < other.m_CurrentElement;
        }

        /** @brief Dereference operator. */
        constexpr RefType operator*() const noexcept { return m_Map[m_CurrentBlock]->Elements[m_CurrentElement]; }
        /** @brief Member access operator. */
        constexpr PtrType operator->() const noexcept { return &m_Map[m_CurrentBlock]->Elements[m_CurrentElement]; }
        /** @brief Subscript offset access operator. */
        constexpr RefType operator[](DiffType offset) const noexcept { return *(*this + offset); }

    private:
        BlockType** m_Map;           ///< Pointer to block map table.
        usize m_CurrentBlock;        ///< Current block index in map.
        usize m_CurrentElement;      ///< Current element offset within block.
    };

    /**
     * @struct DequeBlock
     * @brief Fixed-size memory block storing a chunk of deque elements.
     * @tparam T Element type.
     * @tparam ElementCount Number of elements per block.
     */
    template<typename T, usize ElementCount>
    struct DequeBlock
    {
        T Elements[ElementCount]; ///< Fixed array of elements in this block.
    };

    /**
     * @class Deque
     * @brief Double-ended queue with indexed chunked block allocation.
     *
     * `Deque<T, ElementInBlockCount, Allocator>` provides O(1) insertions and deletions
     * at both the beginning and end, combined with constant-time random access (O(1)) by index.
     *
     * Unlike `MC::Array`, the storage of a deque is not guaranteed to be contiguous; it is segmented
     * into fixed-size blocks pointed to by a dynamic map table. This avoids large reallocation costs
     * and keeps element addresses stable across expansions.
     *
     * @tparam T Stored element type.
     * @tparam ElementInBlockCount Number of elements per allocated block chunk. Defaults to 16.
     * @tparam Allocator Memory allocator type satisfying `Concept::HeapAllocatorType`. Defaults to `Memory::HeapAllocator<void>`.
     *
     * @code
     * MC::Deque<int> dq;
     * dq.PushBack(10);
     * dq.PushFront(5);
     * dq.PushBack(20);
     *
     * // Access by index: dq[0] == 5, dq[1] == 10, dq[2] == 20
     * dq.PopFront();
     * dq.PopBack();
     * @endcode
     */
    template<typename T, usize ElementInBlockCount = 16, Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<void>>
        requires (ElementInBlockCount > 0)
    class Deque
    {
    public:
        using ValueType = T;                                           ///< Element type.
        using PtrType = ValueType*;                                    ///< Pointer to element.
        using ConstPtrType = const ValueType*;                          ///< Pointer to const element.
        using RefType = ValueType&;                                    ///< Reference to element.
        using ConstRefType = const ValueType&;                          ///< Reference to const element.

        using BlockType = DequeBlock<ValueType, ElementInBlockCount>;   ///< Internal block chunk type.
        using AllocatorType = Allocator;                               ///< Base allocator type.
        using BlockAllocatorType = ExtractRebind<AllocatorType, BlockType>;       ///< Allocator for individual blocks.
        using BlockPoolAllocatorType = ExtractRebind<AllocatorType, BlockType*>;   ///< Allocator for map table of block pointers.

    public:
        static constexpr usize INITIAL_BLOCK_COUNT = 8;                ///< Initial number of blocks in the map table.
        static constexpr usize RESIZE_MULTIPLIER = 2;                  ///< Map growth factor.
        static constexpr usize ELEMCNT_IN_BLOCK_COUNT = ElementInBlockCount; ///< Elements per chunk block.

    public:
        using Iterator = DequeIterator<Deque, false>;                   ///< Random-access iterator.
        using ConstIterator = DequeIterator<Deque, true>;               ///< Const random-access iterator.
        using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;///< Reverse iterator.
        using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>; ///< Const reverse iterator.

    public:
        /** @brief Default constructor. Constructs an empty deque with zero allocated blocks. */
        Deque() noexcept = default;

        /**
         * @brief Constructs a deque containing @p count copies of @p value.
         * @param count Number of elements to insert.
         * @param value Value to replicate.
         */
        Deque(usize count, const ValueType& value) noexcept
        {
            for (usize i = 0; i < count; ++i) PushBack(value);
        }

        /**
         * @brief Constructs a deque from input range `[first, last)`.
         * @tparam IteratorType Input iterator type.
         * @param first Range beginning.
         * @param last Range end.
         */
        template<Concept::InputIteratorType IteratorType>
        Deque(IteratorType first, IteratorType last) noexcept
        {
            for (; first != last; ++first) EmplaceBack(*first);
        }

        /**
         * @brief Constructs a deque from an initializer list.
         * @param list Initializer list.
         */
        Deque(Utility::InitializerList<ValueType> list) noexcept
        {
            for (const auto& item : list) PushBack(item);
        }

        /**
         * @brief Copy constructor. Deep copies all elements from @p other.
         * @param other Source deque.
         */
        Deque(const Deque& other) noexcept
            : m_BlockAllocator(other.m_BlockAllocator), m_BlockPoolAllocator(other.m_BlockPoolAllocator)
        {
            CopyFrom(other);
        }

        /**
         * @brief Move constructor. Transfers block map ownership from @p other.
         * @param other Source deque. Leaves @p other in an empty state.
         */
        Deque(Deque&& other) noexcept
            : m_BlockAllocator(Move(other.m_BlockAllocator)), m_BlockPoolAllocator(Move(other.m_BlockPoolAllocator))
        {
            StealFrom(other);
        }

        /** @brief Destructor. Destroys all elements and deallocates all blocks and map tables. */
        ~Deque() noexcept { Release(); }

        /**
         * @brief Copy assignment operator.
         * @param other Source deque.
         * @return Reference to this deque.
         */
        Deque& operator=(const Deque& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_BlockAllocator = other.m_BlockAllocator;
                m_BlockPoolAllocator = other.m_BlockPoolAllocator;
                CopyFrom(other);
            }
            return *this;
        }

        /**
         * @brief Move assignment operator.
         * @param other Source deque.
         * @return Reference to this deque.
         */
        Deque& operator=(Deque&& other) noexcept
        {
            if (this != &other)
            {
                Release();
                m_BlockAllocator = Move(other.m_BlockAllocator);
                m_BlockPoolAllocator = Move(other.m_BlockPoolAllocator);
                StealFrom(other);
            }
            return *this;
        }

    public:
        /** @brief Returns the total number of elements stored. */
        usize Size() const noexcept { return m_ElementCount; }
        /** @brief Checks whether the deque contains no elements. */
        bool Empty() const noexcept { return m_ElementCount == 0; }

        /**
         * @brief Accesses element at @p index with bounds checking.
         * @param index 0-based element position.
         * @return Mutable reference to element.
         */
        RefType At(usize index) noexcept { return const_cast<RefType>(static_cast<const Deque&>(*this).At(index)); }

        /**
         * @brief Accesses element at @p index with bounds checking (const overload).
         * @param index 0-based element position.
         * @return Const reference to element.
         */
        ConstRefType At(usize index) const noexcept
        {
            MOON_CHECK(index < m_ElementCount, "Index in deque is out of range!");
            const usize absoluteIndex = m_FrontElement + index;
            return m_Blocks[m_FrontBlock + absoluteIndex / ELEMCNT_IN_BLOCK_COUNT]->Elements[absoluteIndex % ELEMCNT_IN_BLOCK_COUNT];
        }

        /** @brief Subscript access operator. */
        RefType operator[](usize index) noexcept { return At(index); }
        /** @brief Subscript access operator (const overload). */
        ConstRefType operator[](usize index) const noexcept { return At(index); }

        /** @brief Accesses the first element. */
        RefType Front() noexcept { return const_cast<RefType>(static_cast<const Deque&>(*this).Front()); }
        /** @brief Accesses the first element (const overload). */
        ConstRefType Front() const noexcept
        {
            MOON_CHECK(!Empty(), "Front() on empty deque!");
            return m_Blocks[m_FrontBlock]->Elements[m_FrontElement];
        }

        /** @brief Accesses the last element. */
        RefType Back() noexcept { return const_cast<RefType>(static_cast<const Deque&>(*this).Back()); }
        /** @brief Accesses the last element (const overload). */
        ConstRefType Back() const noexcept
        {
            MOON_CHECK(!Empty(), "Back() on empty deque!");
            if (m_BackElement == 0)
                return m_Blocks[m_BackBlock - 1]->Elements[ELEMCNT_IN_BLOCK_COUNT - 1];
            return m_Blocks[m_BackBlock]->Elements[m_BackElement - 1];
        }

    public:
        /** @brief Returns an iterator to the beginning. */
        Iterator Begin() noexcept { return Iterator(m_Blocks, m_FrontBlock, m_FrontElement); }
        /** @brief Returns a const iterator to the beginning. */
        ConstIterator Begin() const noexcept { return ConstIterator(m_Blocks, m_FrontBlock, m_FrontElement); }
        /** @brief Returns a const iterator to the beginning. */
        ConstIterator CBegin() const noexcept { return Begin(); }
        /** @brief Returns an iterator to the end. */
        Iterator End() noexcept { return Iterator(m_Blocks, m_BackBlock, m_BackElement); }
        /** @brief Returns a const iterator to the end. */
        ConstIterator End() const noexcept { return ConstIterator(m_Blocks, m_BackBlock, m_BackElement); }
        /** @brief Returns a const iterator to the end. */
        ConstIterator CEnd() const noexcept { return End(); }

        /** @brief Returns a reverse iterator to the beginning of reversed deque. */
        ReverseIterator RBegin() noexcept { return ReverseIterator(End()); }
        /** @brief Returns a const reverse iterator to the beginning of reversed deque. */
        ConstReverseIterator CRBegin() const noexcept { return ConstReverseIterator(End()); }
        /** @brief Returns a reverse iterator to the end of reversed deque. */
        ReverseIterator REnd() noexcept { return ReverseIterator(Begin()); }
        /** @brief Returns a const reverse iterator to the end of reversed deque. */
        ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(Begin()); }

        /** @brief Range-for compatible begin iterator. */
        Iterator begin() noexcept { return Begin(); }
        /** @brief Range-for compatible const begin iterator. */
        ConstIterator begin() const noexcept { return Begin(); }
        /** @brief Range-for compatible const begin iterator. */
        ConstIterator cbegin() const noexcept { return CBegin(); }
        /** @brief Range-for compatible end iterator. */
        Iterator end() noexcept { return End(); }
        /** @brief Range-for compatible const end iterator. */
        ConstIterator end() const noexcept { return End(); }
        /** @brief Range-for compatible const end iterator. */
        ConstIterator cend() const noexcept { return CEnd(); }
        /** @brief Range-for compatible reverse begin iterator. */
        ReverseIterator rbegin() noexcept { return RBegin(); }
        /** @brief Range-for compatible const reverse begin iterator. */
        ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
        /** @brief Range-for compatible reverse end iterator. */
        ReverseIterator rend() noexcept { return REnd(); }
        /** @brief Range-for compatible const reverse end iterator. */
        ConstReverseIterator crend() const noexcept { return CREnd(); }

    public:
        /**
         * @brief Appends a copy of @p value to the back.
         * @param value Element to copy.
         * @note O(1) time complexity.
         */
        void PushBack(const ValueType& value) noexcept { EmplaceBack(value); }

        /**
         * @brief Appends @p value to the back by moving.
         * @param value Element to move.
         * @note O(1) time complexity.
         */
        void PushBack(ValueType&& value) noexcept { EmplaceBack(Move(value)); }

        /**
         * @brief Constructs an element in-place at the back.
         * @tparam ArgTypes Constructor argument types.
         * @param arguments Forwarded constructor arguments.
         * @return Reference to created element.
         * @note O(1) time complexity.
         */
        template<typename... ArgTypes>
        RefType EmplaceBack(ArgTypes&&... arguments) noexcept
        {
            if (!m_Blocks) [[unlikely]] InitializeMap();

            PtrType slot = &m_Blocks[m_BackBlock]->Elements[m_BackElement];
            Memory::Construct(slot, Forward<ArgTypes>(arguments)...);
            ++m_ElementCount;

            if (++m_BackElement == ELEMCNT_IN_BLOCK_COUNT)
            {
                m_BackElement = 0;
                AdvanceBackBlock();
            }
            return *slot;
        }

        /**
         * @brief Prepends a copy of @p value to the front.
         * @param value Element to copy.
         * @note O(1) time complexity.
         */
        void PushFront(const ValueType& value) noexcept { EmplaceFront(value); }

        /**
         * @brief Prepends @p value to the front by moving.
         * @param value Element to move.
         * @note O(1) time complexity.
         */
        void PushFront(ValueType&& value) noexcept { EmplaceFront(Move(value)); }

        /**
         * @brief Constructs an element in-place at the front.
         * @tparam ArgTypes Constructor argument types.
         * @param arguments Forwarded constructor arguments.
         * @return Reference to created element.
         * @note O(1) time complexity.
         */
        template<typename... ArgTypes>
        RefType EmplaceFront(ArgTypes&&... arguments) noexcept
        {
            if (!m_Blocks) [[unlikely]] InitializeMap();

            if (m_FrontElement == 0)
            {
                RetreatFrontBlock();
                m_FrontElement = ELEMCNT_IN_BLOCK_COUNT - 1;
            }
            else
                --m_FrontElement;

            PtrType slot = &m_Blocks[m_FrontBlock]->Elements[m_FrontElement];
            Memory::Construct(slot, Forward<ArgTypes>(arguments)...);
            ++m_ElementCount;
            return *slot;
        }

        /**
         * @brief Removes the last element from the deque.
         * @note Asserts non-empty deque in debug builds. O(1) time complexity.
         */
        void PopBack() noexcept
        {
            MOON_CHECK(!Empty(), "PopBack() on empty deque!");
            if (m_BackElement == 0)
            {
                --m_BackBlock;
                m_BackElement = ELEMCNT_IN_BLOCK_COUNT - 1;
            }
            else
                --m_BackElement;

            Memory::Destroy(&m_Blocks[m_BackBlock]->Elements[m_BackElement]);
            --m_ElementCount;
        }

        /**
         * @brief Removes the first element from the deque.
         * @note Asserts non-empty deque in debug builds. O(1) time complexity.
         */
        void PopFront() noexcept
        {
            MOON_CHECK(!Empty(), "PopFront() on empty deque!");
            Memory::Destroy(&m_Blocks[m_FrontBlock]->Elements[m_FrontElement]);
            --m_ElementCount;

            if (++m_FrontElement == ELEMCNT_IN_BLOCK_COUNT)
            {
                m_FrontElement = 0;
                ++m_FrontBlock;
            }
        }

        /**
         * @brief Destroys all elements and resets size to 0 while retaining the center block.
         */
        void Clear() noexcept
        {
            if (!m_Blocks) return;
            DestroyElements();

            const usize middle = m_BlockCount / 2;
            if (!m_Blocks[middle]) m_Blocks[middle] = m_BlockAllocator.Allocate();
            m_FrontBlock = m_BackBlock = middle;
            m_FrontElement = m_BackElement = 0;
            m_ElementCount = 0;
        }

        /**
         * @brief Deallocates unused blocks outside the active range `[m_FrontBlock, m_BackBlock]`.
         */
        void Shrink() noexcept
        {
            if (m_Blocks) FreeSpareBlocks();
        }

    private:
        void InitializeMap() noexcept
        {
            m_BlockCount = INITIAL_BLOCK_COUNT;
            m_Blocks = AllocateMap(m_BlockCount);
            m_FrontBlock = m_BackBlock = m_BlockCount / 2;
            m_FrontElement = m_BackElement = 0;
            m_Blocks[m_FrontBlock] = m_BlockAllocator.Allocate();
        }

        void AdvanceBackBlock() noexcept
        {
            if (m_BackBlock + 1 == m_BlockCount)
                MakeRoomInMap();

            ++m_BackBlock;
            if (!m_Blocks[m_BackBlock])
                m_Blocks[m_BackBlock] = m_BlockAllocator.Allocate();
        }

        void RetreatFrontBlock() noexcept
        {
            if (m_FrontBlock == 0)
                MakeRoomInMap();

            --m_FrontBlock;
            if (!m_Blocks[m_FrontBlock])
                m_Blocks[m_FrontBlock] = m_BlockAllocator.Allocate();
        }

        void MakeRoomInMap() noexcept
        {
            FreeSpareBlocks();

            const usize usedBlocks = m_BackBlock - m_FrontBlock + 1;

            if (m_BlockCount >= 2 * (usedBlocks + 1))
            {
                const usize newFrontBlock = (m_BlockCount - usedBlocks) / 2;
                memmove(m_Blocks + newFrontBlock, m_Blocks + m_FrontBlock, usedBlocks * sizeof(BlockType*));
                for (usize i = 0; i < newFrontBlock; ++i) m_Blocks[i] = nullptr;
                for (usize i = newFrontBlock + usedBlocks; i < m_BlockCount; ++i) m_Blocks[i] = nullptr;
                RebaseBlocks(newFrontBlock);
                return;
            }

            usize newBlockCount = m_BlockCount * RESIZE_MULTIPLIER;
            if (newBlockCount < usedBlocks + 2) newBlockCount = usedBlocks + 2;

            BlockType** newBlocks = AllocateMap(newBlockCount);
            const usize newFrontBlock = (newBlockCount - usedBlocks) / 2;
            memcpy(newBlocks + newFrontBlock, m_Blocks + m_FrontBlock, usedBlocks * sizeof(BlockType*));
            m_BlockPoolAllocator.Deallocate(m_Blocks, m_BlockCount);

            m_Blocks = newBlocks;
            m_BlockCount = newBlockCount;
            RebaseBlocks(newFrontBlock);
        }

        void RebaseBlocks(usize newFrontBlock) noexcept
        {
            m_BackBlock = newFrontBlock + (m_BackBlock - m_FrontBlock);
            m_FrontBlock = newFrontBlock;
        }

        BlockType** AllocateMap(usize blockCount) noexcept
        {
            BlockType** map = m_BlockPoolAllocator.Allocate(blockCount);
            MOON_CHECK(map, "Deque block map allocation failed!");
            for (usize i = 0; i < blockCount; ++i) map[i] = nullptr;
            return map;
        }

        void FreeSpareBlocks() noexcept
        {
            for (usize i = 0; i < m_FrontBlock; ++i)
                if (m_Blocks[i]) { m_BlockAllocator.Deallocate(m_Blocks[i]); m_Blocks[i] = nullptr; }
            for (usize i = m_BackBlock + 1; i < m_BlockCount; ++i)
                if (m_Blocks[i]) { m_BlockAllocator.Deallocate(m_Blocks[i]); m_Blocks[i] = nullptr; }
        }

        void DestroyElements() noexcept
        {
            if constexpr (!IsTriviallyDestructible<ValueType>)
                for (Iterator it = Begin(), end = End(); it != end; ++it)
                    Memory::Destroy(&*it);
        }

        void Release() noexcept
        {
            if (!m_Blocks) return;
            DestroyElements();
            for (usize i = 0; i < m_BlockCount; ++i)
                if (m_Blocks[i]) m_BlockAllocator.Deallocate(m_Blocks[i]);
            m_BlockPoolAllocator.Deallocate(m_Blocks, m_BlockCount);
            ResetState();
        }

        void CopyFrom(const Deque& other) noexcept
        {
            if (!other.m_Blocks) return;

            m_BlockCount = other.m_BlockCount;
            m_FrontBlock = other.m_FrontBlock;
            m_BackBlock = other.m_BackBlock;
            m_FrontElement = other.m_FrontElement;
            m_BackElement = other.m_BackElement;
            m_Blocks = AllocateMap(m_BlockCount);

            for (usize b = m_FrontBlock; b <= m_BackBlock; ++b)
            {
                m_Blocks[b] = m_BlockAllocator.Allocate();
                const usize from = (b == m_FrontBlock) ? m_FrontElement : 0;
                const usize to = (b == m_BackBlock) ? m_BackElement : ELEMCNT_IN_BLOCK_COUNT; // exclusive

                if constexpr (IsTriviallyCopyable<ValueType>)
                {
                    if (to > from)
                        memcpy(m_Blocks[b]->Elements + from, other.m_Blocks[b]->Elements + from, (to - from) * sizeof(ValueType));
                }
                else
                {
                    for (usize u = from; u < to; ++u)
                        Memory::Construct(&m_Blocks[b]->Elements[u], other.m_Blocks[b]->Elements[u]);
                }
            }
            m_ElementCount = other.m_ElementCount;
        }

        void StealFrom(Deque& other) noexcept
        {
            m_Blocks = other.m_Blocks;
            m_BlockCount = other.m_BlockCount;
            m_FrontBlock = other.m_FrontBlock;
            m_BackBlock = other.m_BackBlock;
            m_FrontElement = other.m_FrontElement;
            m_BackElement = other.m_BackElement;
            m_ElementCount = other.m_ElementCount;
            other.ResetState();
        }

        void ResetState() noexcept
        {
            m_Blocks = nullptr;
            m_BlockCount = m_FrontBlock = m_BackBlock = 0;
            m_FrontElement = m_BackElement = m_ElementCount = 0;
        }

    private:
        BlockType** m_Blocks = nullptr;   ///< Array of pointers to blocks.
        usize m_BlockCount = 0;           ///< Capacity of pointer map table.
        usize m_FrontBlock = 0;           ///< Index of block holding front element.
        usize m_BackBlock = 0;            ///< Index of block holding back insertion position.

        usize m_FrontElement = 0;         ///< Element offset of front element inside m_FrontBlock.
        usize m_BackElement = 0;          ///< Element offset of next back insertion slot inside m_BackBlock.
        usize m_ElementCount = 0;         ///< Total number of active elements in deque.

        BlockAllocatorType m_BlockAllocator;       ///< Allocator for blocks.
        BlockPoolAllocatorType m_BlockPoolAllocator;///< Allocator for pointer map table.
    };
}
