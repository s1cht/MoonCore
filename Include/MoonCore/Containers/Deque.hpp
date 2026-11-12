#pragma once
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/Iterator.hpp"

namespace MC
{
    template<typename Container, bool IsConst = false>
    class DequeIterator
    {
    private:
        static constexpr usize ELEMCNT_IN_BLOCK_COUNT = Container::ELEMCNT_IN_BLOCK_COUNT;
        friend class DequeIterator<Container, !IsConst>;

    public:
        using ValueType = typename Container::ValueType;
        using DiffType = iptr;
        using PtrType = Conditional<IsConst, const ValueType*, ValueType*>;
        using RefType = Conditional<IsConst, const ValueType&, ValueType&>;
        using Category = Iterator::Tags::RandomAccess;
        using BlockType = typename Container::BlockType;

    public:
        constexpr DequeIterator() noexcept
            : m_Map(nullptr), m_CurrentBlock(0), m_CurrentElement(0)
        {}
        constexpr DequeIterator(BlockType** map, usize block, usize element) noexcept
            : m_Map(map), m_CurrentBlock(block), m_CurrentElement(element)
        {}

        template<bool OtherConst> requires (IsConst && !OtherConst)
            constexpr DequeIterator(const DequeIterator<Container, OtherConst>& other) noexcept
            : m_Map(other.m_Map), m_CurrentBlock(other.m_CurrentBlock), m_CurrentElement(other.m_CurrentElement)
        {}

    public:
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
        constexpr DequeIterator& operator-=(DiffType offset) noexcept { return *this += -offset; }

        constexpr DequeIterator operator+(DiffType offset) const noexcept { DequeIterator r = *this; r += offset; return r; }
        constexpr DequeIterator operator-(DiffType offset) const noexcept { DequeIterator r = *this; r -= offset; return r; }
        friend constexpr DequeIterator operator+(DiffType offset, const DequeIterator& it) noexcept { return it + offset; }

        constexpr DequeIterator& operator++() noexcept
        {
            if (++m_CurrentElement == ELEMCNT_IN_BLOCK_COUNT)
            {
                m_CurrentElement = 0;
                ++m_CurrentBlock;
            }
            return *this;
        }
        constexpr DequeIterator operator++(int) noexcept { DequeIterator t = *this; ++(*this); return t; }

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
        constexpr DequeIterator operator--(int) noexcept { DequeIterator t = *this; --(*this); return t; }

        constexpr DiffType operator-(const DequeIterator& other) const noexcept
        {
            DiffType blockDiff = static_cast<DiffType>(m_CurrentBlock) - static_cast<DiffType>(other.m_CurrentBlock);
            DiffType elementDiff = static_cast<DiffType>(m_CurrentElement) - static_cast<DiffType>(other.m_CurrentElement);
            return blockDiff * static_cast<DiffType>(ELEMCNT_IN_BLOCK_COUNT) + elementDiff;
        }

        constexpr bool operator==(const DequeIterator& other) const noexcept
        {
            return m_CurrentBlock == other.m_CurrentBlock && m_CurrentElement == other.m_CurrentElement;
        }
        constexpr bool operator<(const DequeIterator& other) const noexcept
        {
            if (m_CurrentBlock != other.m_CurrentBlock)
                return m_CurrentBlock < other.m_CurrentBlock;
            return m_CurrentElement < other.m_CurrentElement;
        }
        constexpr bool operator>(const DequeIterator& other) const noexcept { return other < *this; }
        constexpr bool operator<=(const DequeIterator& other) const noexcept { return !(other < *this); }
        constexpr bool operator>=(const DequeIterator& other) const noexcept { return !(*this < other); }

        constexpr RefType operator[](DiffType offset) const noexcept { return *(*this + offset); }
        constexpr RefType operator*() const noexcept { return m_Map[m_CurrentBlock]->Elements[m_CurrentElement]; }
        constexpr PtrType operator->() const noexcept { return &m_Map[m_CurrentBlock]->Elements[m_CurrentElement]; }

    private:
        BlockType** m_Map;
        usize m_CurrentBlock;
        usize m_CurrentElement;
    };

    template<typename T, usize ElementCount>
    struct DequeBlock
    {
        T Elements[ElementCount];
    };

    template<typename T, usize ElementInBlockCount = 16, Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<void>>
        requires (ElementInBlockCount > 0)
    class Deque
    {
    public:
        using ValueType = T;
        using PtrType = ValueType*;
        using ConstPtrType = const ValueType*;
        using RefType = ValueType&;
        using ConstRefType = const ValueType&;

        using BlockType = DequeBlock<ValueType, ElementInBlockCount>;
        using AllocatorType = Allocator;
        using BlockAllocatorType = ExtractRebind<AllocatorType, BlockType>;
        using BlockPoolAllocatorType = ExtractRebind<AllocatorType, BlockType*>;

    public:
        static constexpr usize INITIAL_BLOCK_COUNT = 8;
        static constexpr usize RESIZE_MULTIPLIER = 2;
        static constexpr usize ELEMCNT_IN_BLOCK_COUNT = ElementInBlockCount;

    public:
        using Iterator = DequeIterator<Deque, false>;
        using ConstIterator = DequeIterator<Deque, true>;
        using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;
        using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>;

    public:
        Deque() noexcept = default;

        Deque(usize count, const ValueType& value) noexcept
        {
            for (usize i = 0; i < count; ++i) PushBack(value);
        }

        template<Concept::InputIteratorType IteratorType>
        Deque(IteratorType first, IteratorType last) noexcept
        {
            for (; first != last; ++first) EmplaceBack(*first);
        }

        Deque(Utility::InitializerList<ValueType> list) noexcept
        {
            for (const auto& item : list) PushBack(item);
        }

        Deque(const Deque& other) noexcept
            : m_BlockAllocator(other.m_BlockAllocator), m_BlockPoolAllocator(other.m_BlockPoolAllocator)
        {
            CopyFrom(other);
        }

        Deque(Deque&& other) noexcept
            : m_BlockAllocator(Move(other.m_BlockAllocator)), m_BlockPoolAllocator(Move(other.m_BlockPoolAllocator))
        {
            StealFrom(other);
        }

        ~Deque() noexcept { Release(); }

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
        usize Size() const noexcept { return m_ElementCount; }
        bool Empty() const noexcept { return m_ElementCount == 0; }

        RefType At(usize index) noexcept { return const_cast<RefType>(static_cast<const Deque&>(*this).At(index)); }
        ConstRefType At(usize index) const noexcept
        {
            MC_CHECK(index < m_ElementCount, "Index in deque is out of range!");
            const usize absoluteIndex = m_FrontElement + index;
            return m_Blocks[m_FrontBlock + absoluteIndex / ELEMCNT_IN_BLOCK_COUNT]->Elements[absoluteIndex % ELEMCNT_IN_BLOCK_COUNT];
        }
        RefType operator[](usize index) noexcept { return At(index); }
        ConstRefType operator[](usize index) const noexcept { return At(index); }

        RefType Front() noexcept { return const_cast<RefType>(static_cast<const Deque&>(*this).Front()); }
        ConstRefType Front() const noexcept
        {
            MC_CHECK(!Empty(), "Front() on empty deque!");
            return m_Blocks[m_FrontBlock]->Elements[m_FrontElement];
        }

        RefType Back() noexcept { return const_cast<RefType>(static_cast<const Deque&>(*this).Back()); }
        ConstRefType Back() const noexcept
        {
            MC_CHECK(!Empty(), "Back() on empty deque!");
            if (m_BackElement == 0)
                return m_Blocks[m_BackBlock - 1]->Elements[ELEMCNT_IN_BLOCK_COUNT - 1];
            return m_Blocks[m_BackBlock]->Elements[m_BackElement - 1];
        }

    public:
        Iterator Begin() noexcept { return Iterator(m_Blocks, m_FrontBlock, m_FrontElement); }
        ConstIterator Begin() const noexcept { return ConstIterator(m_Blocks, m_FrontBlock, m_FrontElement); }
        ConstIterator CBegin() const noexcept { return Begin(); }
        Iterator End() noexcept { return Iterator(m_Blocks, m_BackBlock, m_BackElement); }
        ConstIterator End() const noexcept { return ConstIterator(m_Blocks, m_BackBlock, m_BackElement); }
        ConstIterator CEnd() const noexcept { return End(); }

        ReverseIterator RBegin() noexcept { return ReverseIterator(End()); }
        ConstReverseIterator CRBegin() const noexcept { return ConstReverseIterator(End()); }
        ReverseIterator REnd() noexcept { return ReverseIterator(Begin()); }
        ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(Begin()); }

        Iterator begin() noexcept { return Begin(); }
        ConstIterator begin() const noexcept { return Begin(); }
        ConstIterator cbegin() const noexcept { return CBegin(); }
        Iterator end() noexcept { return End(); }
        ConstIterator end() const noexcept { return End(); }
        ConstIterator cend() const noexcept { return CEnd(); }
        ReverseIterator rbegin() noexcept { return RBegin(); }
        ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
        ReverseIterator rend() noexcept { return REnd(); }
        ConstReverseIterator crend() const noexcept { return CREnd(); }

    public:
        void PushBack(const ValueType& value) noexcept { EmplaceBack(value); }
        void PushBack(ValueType&& value) noexcept { EmplaceBack(Move(value)); }

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

        void PushFront(const ValueType& value) noexcept { EmplaceFront(value); }
        void PushFront(ValueType&& value) noexcept { EmplaceFront(Move(value)); }

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

        void PopBack() noexcept
        {
            MC_CHECK(!Empty(), "PopBack() on empty deque!");
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

        void PopFront() noexcept
        {
            MC_CHECK(!Empty(), "PopFront() on empty deque!");
            Memory::Destroy(&m_Blocks[m_FrontBlock]->Elements[m_FrontElement]);
            --m_ElementCount;

            if (++m_FrontElement == ELEMCNT_IN_BLOCK_COUNT)
            {
                m_FrontElement = 0;
                ++m_FrontBlock;
            }
        }

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
            MC_CHECK(map, "Deque block map allocation failed!");
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
        BlockType** m_Blocks = nullptr;
        usize m_BlockCount = 0;
        usize m_FrontBlock = 0;
        usize m_BackBlock = 0;

        usize m_FrontElement = 0;
        usize m_BackElement = 0;
        usize m_ElementCount = 0;

        BlockAllocatorType m_BlockAllocator;
        BlockPoolAllocatorType m_BlockPoolAllocator;
    };
}