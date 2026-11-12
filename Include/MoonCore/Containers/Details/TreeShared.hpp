#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Callables/Functors.hpp"
#include "MoonCore/Utility/Pair.hpp"

namespace MC::Concept
{
    template<typename T> concept TreeType = IsDefaultConstructible<T> && IsNothrowMoveConstructible<T>;
    template<typename T> concept TreeMappedType = TreeType<T> || IsVoid<T>;
}

namespace MC
{
    template<typename Key, typename Value>
    struct TreeTraits
    {
        using StoredType = Utility::Pair<const Key, Value>;
        using ValueType = StoredType;
        static constexpr bool IsSet = false;

        static const Key& KeyOf(const StoredType& value) noexcept { return value.Key; }
    };

    template<typename Key>
    struct TreeTraits<Key, void>
    {
        using StoredType = Key;
        using ValueType = const Key;
        static constexpr bool IsSet = true;

        static const Key& KeyOf(const StoredType& value) noexcept { return value; }
    };

    template <typename ValueType>
    struct TreeBranch
    {
        ValueType Value;
        u32 Height;
        // Pointers
        TreeBranch* Parent;
        TreeBranch* LeftPart;
        TreeBranch* RightPart;

        TreeBranch* Successor() noexcept
        {
            TreeBranch* next = this;
            if (RightPart)
            {
                next = RightPart;
                while (next->LeftPart) next = next->LeftPart;
                return next;

            }
            while (next->Parent && next->Parent->RightPart == next) next = next->Parent;
            return next->Parent;
        }

        const TreeBranch* Successor() const noexcept
        {
            const TreeBranch* next = this;
            if (RightPart)
            {
                next = RightPart;
                while (next->LeftPart) next = next->LeftPart;
                return next;

            }
            while (next->Parent && next->Parent->RightPart == next) next = next->Parent;
            return next->Parent;
        }

        TreeBranch* Precessor() noexcept
        {
            TreeBranch* last = this;
            if (LeftPart)
            {
                last = LeftPart;
                while (last->RightPart) last = last->RightPart;
                return last;
            }
            while (last->Parent && last->Parent->LeftPart == last) last = last->Parent;
            return last->Parent;
        }

        const TreeBranch* Precessor() const noexcept
        {
            const TreeBranch* last = this;
            if (LeftPart)
            {
                last = LeftPart;
                while (last->RightPart) last = last->RightPart;
                return last;
            }
            while (last->Parent && last->Parent->LeftPart == last) last = last->Parent;
            return last->Parent;
        }
    };

    template<typename Container, bool IsConst = false>
    class TreeIterator
    {
    public:
        using KeyType = typename Container::KeyType;
        using ValueType = typename Container::ValueType;
        using DiffType = iptr;
        using PtrType = Conditional<IsConst, const ValueType*, ValueType*>;
        using RefType = Conditional<IsConst, const ValueType&, ValueType&>;
        using Category = Iterator::Tags::Bidirectional;
        using BranchPtrType = typename Container::BranchType*;

    public:
        constexpr TreeIterator() noexcept : m_Ptr(nullptr) {}
        constexpr TreeIterator(BranchPtrType ptr) noexcept : m_Ptr(ptr) {}

        constexpr TreeIterator(const TreeIterator& other) noexcept { m_Ptr = other.m_Ptr; }
        constexpr TreeIterator(TreeIterator&& other) noexcept { m_Ptr = other.m_Ptr; }
    public:
        constexpr TreeIterator& operator=(const TreeIterator& other) noexcept { m_Ptr = other.m_Ptr; return *this; }
        constexpr TreeIterator& operator=(TreeIterator&& other) noexcept { m_Ptr = Move(other.m_Ptr); return *this; }

    public:
        constexpr TreeIterator& operator+=(usize index) { for (usize i = 0; i < index; i++) m_Ptr = m_Ptr->Successor(); return *this; }
        constexpr TreeIterator& operator-=(usize index) { for (usize i = 0; i < index; i++) m_Ptr = m_Ptr->Precessor(); return *this; }

        constexpr TreeIterator operator+(usize index) const { return TreeIterator(m_Ptr) += index; }
        constexpr TreeIterator operator-(usize index) const { return TreeIterator(m_Ptr) -= index; }

        constexpr TreeIterator& operator++() { m_Ptr = m_Ptr->Successor(); return *this; }
        constexpr TreeIterator operator++(int) { TreeIterator temp = *this; m_Ptr = m_Ptr->Successor(); return temp; }

        constexpr TreeIterator& operator--() { m_Ptr = m_Ptr->Precessor(); return *this; }
        constexpr TreeIterator operator--(int) { TreeIterator temp = *this; m_Ptr = m_Ptr->Precessor(); return temp; }

        constexpr bool operator==(const TreeIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
        constexpr bool operator!=(const TreeIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }

        constexpr RefType operator[](usize index) const { return (*this + index).m_Ptr->Value; }
        constexpr PtrType operator->() const { return &m_Ptr->Value; }
        constexpr RefType operator*() const { return m_Ptr->Value; }

    public:
        BranchPtrType m_Ptr;
    };

    template <typename Key, Concept::TreeMappedType Value,
        Concept::HeapAllocatorType Allocator,
        Concept::CompareFunctor Compare,
        bool AllowDuplicates>
    class AVLTree
    {
    public:
        using TraitsType = TreeTraits<Key, Value>;

        using KeyType = Key;
        using MappedType = Value;
        using StoredType = typename TraitsType::StoredType;
        using ValueType = typename TraitsType::ValueType;
        using BranchType = TreeBranch<StoredType>;
        using PtrType = ValueType*;
        using ConstPtrType = const ValueType*;
        using RefType = ValueType&;
        using ConstRefType = const ValueType&;

        static constexpr bool IsSet = TraitsType::IsSet;
        static constexpr bool IsMulti = AllowDuplicates;

    public:
        using AllocatorType = Allocator;
        using CompareFunctor = Compare;

        using Iterator = TreeIterator<AVLTree, false>;
        using ConstIterator = TreeIterator<AVLTree, true>;
        using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;
        using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>;

    public:
        constexpr AVLTree() noexcept
            : m_Root(nullptr), m_ElementCount(0), m_Allocator(AllocatorType())
        {}

        AVLTree(const AVLTree& other) noexcept
        {
            m_ElementCount = other.m_ElementCount;
            m_Allocator = other.m_Allocator;
            m_Root = CloneBranch(other.m_Root);
        }

        AVLTree(AVLTree&& other) noexcept
        {
            m_Allocator = other.m_Allocator;
            m_ElementCount = other.m_ElementCount;
            m_Root = other.m_Root;

            other.m_ElementCount = 0;
            other.m_Root = nullptr;
        }

        ~AVLTree() noexcept
        {
            m_ElementCount = 0;
            Clear();
        }

    public:
        usize Size() const noexcept { return m_ElementCount; }
        const BranchType* Data() const noexcept { return m_Root; }
        bool Empty() const noexcept { return m_Root == nullptr; }

        ValueType& Front() const noexcept { return GetLeftmostBranch(m_Root)->Value; }
        ValueType& Back() const noexcept { return GetRightmostBranch(m_Root)->Value; }

    public:
        Iterator Begin() noexcept { return Iterator(GetLeftmostBranch(m_Root)); }
        ConstIterator Begin() const noexcept { return ConstIterator(GetLeftmostBranch(m_Root)); }
        ConstIterator CBegin() const noexcept { return ConstIterator(GetLeftmostBranch(m_Root)); }
        Iterator End() noexcept { return Iterator(nullptr); }
        ConstIterator End() const noexcept { return ConstIterator(nullptr); }
        static ConstIterator CEnd() noexcept { return ConstIterator(nullptr); }

        ReverseIterator RBegin() noexcept { return m_Root != nullptr ? ReverseIterator(GetRightmostBranch(m_Root)) : REnd(); }
        ConstReverseIterator RBegin() const noexcept { return m_Root != nullptr ? ConstReverseIterator(GetRightmostBranch(m_Root)) : REnd(); }
        ConstReverseIterator CRBegin() const noexcept { return m_Root != nullptr ? ConstReverseIterator(GetRightmostBranch(m_Root)) : CREnd(); }
        ReverseIterator REnd() noexcept { return ReverseIterator(nullptr); }
        ConstReverseIterator REnd() const noexcept { return ConstReverseIterator(nullptr); }
        static ConstReverseIterator CREnd() noexcept { return ConstReverseIterator(nullptr); }

    public:
        Iterator begin() noexcept { return Begin(); }
        ConstIterator begin() const noexcept { return Begin(); }
        Iterator end() noexcept { return End(); }
        ConstIterator end() const noexcept { return End(); }

    public:
        AVLTree& operator=(const AVLTree& other) noexcept
        {
            if (this != &other)
            {
                Clear();
                m_ElementCount = other.m_ElementCount;
                m_Allocator = other.m_Allocator;
                m_Root = CloneBranch(other.m_Root);
            }
            return *this;
        }

        AVLTree& operator=(AVLTree&& other) noexcept
        {
            if (this != &other)
            {
                Clear();
                m_Allocator = other.m_Allocator;
                m_ElementCount = other.m_ElementCount;
                m_Root = other.m_Root;

                other.m_ElementCount = 0;
                other.m_Root = nullptr;
            }
            return *this;
        }

        template<typename SameMappedType = MappedType>
            requires IsSame<SameMappedType, MappedType>
        SameMappedType& operator[](const KeyType& key) noexcept
        {
            static_assert(!AllowDuplicates && !IsSet, "At() is only accessible in Map!");
            if (BranchType* node = SearchBranch(key)) return node->Value.Value;
            Iterator emplaced = EmplaceImpl({ key, MappedType() }).Key;
            return emplaced->Value;
        }

    public:
        template<typename ConvertibleMappedType = MappedType>
            requires IsConvertible<ConvertibleMappedType, MappedType>
        Utility::Pair<Iterator, bool> Insert(const KeyType& key, const ConvertibleMappedType& value) noexcept
        {
            static_assert(!IsSet, "Insert(Key, Value) is only accessible in Maps!");
            return EmplaceImpl(ValueType( key, static_cast<const MappedType&>(value) ));
        }
        template<typename ConvertibleMappedType = MappedType>
            requires IsConvertible<ConvertibleMappedType, MappedType>
        Utility::Pair<Iterator, bool> Insert(KeyType&& key, ConvertibleMappedType&& value) noexcept
        {
            static_assert(!IsSet, "Insert(Key, Value) is only accessible in Maps!");
            return EmplaceImpl(StoredType(Move(key), Move(static_cast<MappedType&&>(value))));
        }
        Utility::Pair<Iterator, bool> Insert(const StoredType& value) noexcept { return EmplaceImpl(StoredType(value)); }
        Utility::Pair<Iterator, bool> Insert(StoredType&& value) noexcept { return EmplaceImpl(Forward<StoredType>(value)); }

        template<typename... ArgTypes>
        Utility::Pair<Iterator, bool> Emplace(ArgTypes&&... args) noexcept
        {
            return EmplaceOrAssignImpl(StoredType{ Forward<ArgTypes>(args)... });
        }

        auto Erase(const KeyType& key) noexcept
        {
            if constexpr (!AllowDuplicates)
            {
                BranchType* branch = FindBranch(key);
                return branch ? EraseImpl(branch) : End();
            }
            else
            {
                usize removed = 0;
                Iterator it = LowerBound(key);
                while (it.m_Ptr && !m_Compare(key, KeyOf(it.m_Ptr)))
                {
                    it = EraseImpl(it.m_Ptr);
                    ++removed;
                }
                return removed;
            }
        }

        Iterator Erase(Iterator iterator) noexcept { return EraseImpl(iterator.m_Ptr); }
        Iterator Erase(ConstIterator iterator) noexcept { return EraseImpl(iterator.m_Ptr); }
        Iterator Erase(Iterator first, Iterator last) noexcept { while (first != last) first = EraseImpl(first.m_Ptr); return last; }
        Iterator Erase(ConstIterator first, ConstIterator last) noexcept
        {
            while (first != last) first = ConstIterator(EraseImpl(first.m_Ptr).m_Ptr);
            return Iterator(last.m_Ptr);
        }

        bool Contains(const KeyType& key) const noexcept { return FindBranch(key) != nullptr; }

        template<typename SameMappedType = MappedType>
            requires IsSame<SameMappedType, MappedType>
        SameMappedType& At(const KeyType& key) noexcept
        {
            static_assert(!AllowDuplicates && !IsSet, "At() is only accessible in Map!");
            BranchType* node = FindBranch(key);
            MC_VALIDATE(node, "Key doesn't exits in map");
            return node->Value.Value;
        }

        template<typename SameMappedType = MappedType>
            requires IsSame<SameMappedType, MappedType>
        const SameMappedType& At(const KeyType& key) const noexcept
        {
            static_assert(!AllowDuplicates && !IsSet, "At() is only accessible in Map!");
            const BranchType* node = FindBranch(key);
            MC_VALIDATE(node, "Key doesn't exits in map");
            return node->Value.Value;
        }

        void Clear() noexcept
        {
            ClearImpl();
        }

        Iterator Find(const KeyType& key) noexcept { return Iterator(FindBranch(key)); }
        ConstIterator Find(const KeyType& key) const noexcept { return ConstIterator(FindBranch(key)); }

        Iterator LowerBound(const KeyType& key) noexcept { return Iterator(LowerBoundImpl(key)); }
        ConstIterator LowerBound(const KeyType& key) const noexcept { return ConstIterator(LowerBoundImpl(key)); }

        Iterator UpperBound(const KeyType& key) noexcept { return Iterator(UpperBoundImpl(key)); }
        ConstIterator UpperBound(const KeyType& key) const noexcept { return ConstIterator(UpperBoundImpl(key)); }

    private:
        static const KeyType& KeyOf(const BranchType* branch) noexcept { return TraitsType::KeyOf(branch->Value); }

        bool Equivalent(const KeyType& left, const KeyType& right) const noexcept
        {
            return !m_Compare(left, right) && !m_Compare(right, left);
        }

        BranchType* FindBranch(const KeyType& key) const noexcept
        {
            if constexpr (!AllowDuplicates)
            {
                return SearchBranch(key);
            }
            else
            {
                BranchType* branch = LowerBoundImpl(key);
                if (!branch) return nullptr;
                return m_Compare(key, KeyOf(branch)) ? nullptr : branch;
            }
        }

        BranchType* LowerBoundImpl(const KeyType& key) const noexcept
        {
            return m_Root ? LowerBoundImpl(key, m_Root) : nullptr;
        }

        BranchType* LowerBoundImpl(const KeyType& key, BranchType* branch) const noexcept
        {
            if (!m_Compare(KeyOf(branch), key))
            {
                if (!branch->LeftPart)
                    return branch;

                BranchType* result = LowerBoundImpl(key, branch->LeftPart);
                return result == nullptr ? branch : result;
            }

            if (branch->RightPart)
                return LowerBoundImpl(key, branch->RightPart);
            return nullptr;
        }

        BranchType* UpperBoundImpl(const KeyType& key) const noexcept
        {
            return m_Root ? UpperBoundImpl(key, m_Root) : nullptr;
        }

        BranchType* UpperBoundImpl(const KeyType& key, BranchType* branch) const noexcept
        {
            if (m_Compare(key, KeyOf(branch)))
            {
                if (!branch->LeftPart)
                    return branch;

                BranchType* result = UpperBoundImpl(key, branch->LeftPart);
                return result == nullptr ? branch : result;
            }

            if (branch->RightPart)
                return UpperBoundImpl(key, branch->RightPart);
            return nullptr;
        }

        Utility::Pair<Iterator, bool> EmplaceImpl(StoredType&& value) noexcept
        {
            auto Allocate = [this, &value]() noexcept
            {
                ++m_ElementCount;
                return AllocateBranch(Forward<StoredType>(value));
            };

            if (!m_Root)
            {
                m_Root = Allocate();
                return { Iterator(m_Root), true };
            }

            BranchType* currentBranch = m_Root;
            BranchType* allocatedBranch;

            for (;;)
            {
                if (m_Compare(TraitsType::KeyOf(value), KeyOf(currentBranch)))
                {
                    if (!currentBranch->LeftPart)
                    {
                        currentBranch->LeftPart = Allocate();
                        allocatedBranch = currentBranch->LeftPart;
                        break;
                    }
                    currentBranch = currentBranch->LeftPart;
                }
                else if constexpr (!AllowDuplicates)
                {
                    if (!m_Compare(KeyOf(currentBranch), TraitsType::KeyOf(value)))
                    {
                        return { Iterator(currentBranch), false };
                    }
                    else if (!currentBranch->RightPart)
                    {
                        currentBranch->RightPart = Allocate();
                        allocatedBranch = currentBranch->RightPart;
                        break;
                    }
                    else currentBranch = currentBranch->RightPart;
                }
                else
                {
                    if (!currentBranch->RightPart)
                    {
                        currentBranch->RightPart = Allocate();
                        allocatedBranch = currentBranch->RightPart;
                        break;
                    }
                    currentBranch = currentBranch->RightPart;
                }
            }

            allocatedBranch->Parent = currentBranch;
            RebalanceUpwards(currentBranch);

            return { Iterator(allocatedBranch), true };
        }

        Utility::Pair<Iterator, bool> EmplaceOrAssignImpl(StoredType&& value) noexcept
        {
            auto Allocate = [this, &value]() noexcept
            {
                ++m_ElementCount;
                return AllocateBranch(Forward<StoredType>(value));
            };

            if (!m_Root)
            {
                m_Root = Allocate();
                return { Iterator(m_Root), true };
            }

            BranchType* currentBranch = m_Root;
            BranchType* allocatedBranch;

            for (;;)
            {
                if (m_Compare(TraitsType::KeyOf(value), KeyOf(currentBranch)))
                {
                    if (!currentBranch->LeftPart)
                    {
                        currentBranch->LeftPart = Allocate();
                        allocatedBranch = currentBranch->LeftPart;
                        break;
                    }
                    currentBranch = currentBranch->LeftPart;
                }
                else if constexpr (!AllowDuplicates)
                {
                    if (!m_Compare(KeyOf(currentBranch), TraitsType::KeyOf(value)))
                    {
                        if constexpr (!IsSet)
                            currentBranch->Value.Value = Move(value.Value);
                        return { Iterator(currentBranch), false };
                    }
                    else if (!currentBranch->RightPart)
                    {
                        currentBranch->RightPart = Allocate();
                        allocatedBranch = currentBranch->RightPart;
                        break;
                    }
                    else currentBranch = currentBranch->RightPart;
                }
                else
                {
                    if (!currentBranch->RightPart)
                    {
                        currentBranch->RightPart = Allocate();
                        allocatedBranch = currentBranch->RightPart;
                        break;
                    }
                    currentBranch = currentBranch->RightPart;
                }
            }

            allocatedBranch->Parent = currentBranch;
            RebalanceUpwards(currentBranch);

            return { Iterator(allocatedBranch), true };
        }

        Iterator EraseImpl(BranchType* target) noexcept
        {
            BranchType* next = target->Successor();
            BranchType* rebalanceFrom;

            if (target->LeftPart && target->RightPart)
            {
                BranchType* successor = GetLeftmostBranch(target->RightPart);
                rebalanceFrom = (successor->Parent == target) ? successor : successor->Parent;

                if (successor->Parent != target)
                {
                    Transplant(successor, successor->RightPart);
                    successor->RightPart = target->RightPart;
                    successor->RightPart->Parent = successor;
                }

                Transplant(target, successor);
                successor->LeftPart = target->LeftPart;
                successor->LeftPart->Parent = successor;
                successor->Height = target->Height;
            }
            else
            {
                rebalanceFrom = target->Parent;
                Transplant(target, target->LeftPart ? target->LeftPart : target->RightPart);
            }

            DeallocateBranch(target);
            --m_ElementCount;
            RebalanceUpwards(rebalanceFrom);
            return Iterator(next);
        }

        void ClearImpl() noexcept
        {
            BranchType* current = m_Root;
            while (current)
            {
                if (!current->RightPart && !current->LeftPart)
                {
                    BranchType* parent = current->Parent;
                    DeallocateBranch(current);
                    current = parent;
                    continue;
                }
                if (current->RightPart)
                {
                    BranchType* child = current->RightPart;
                    current->RightPart = nullptr;
                    current = child;
                    continue;
                }
                if (current->LeftPart)
                {
                    BranchType* child = current->LeftPart;
                    current->LeftPart = nullptr;
                    current = child;
                }
            }

            m_Root = nullptr;
            m_ElementCount = 0;
        }

    private:
        void Transplant(BranchType* left, BranchType* right) noexcept
        {
            BranchType* parent = left->Parent;
            if (!parent)
                m_Root = right;
            else if (parent->LeftPart == left)
                parent->LeftPart = right;
            else
                parent->RightPart = right;
            if (right)
                right->Parent = parent;
        }

        i32 GetBalanceFactor(BranchType* branch) const noexcept
        {
            return branch == nullptr ? 0
                : static_cast<i32>(GetHeight(branch->LeftPart)) - static_cast<i32>(GetHeight(branch->RightPart));
        }

        static u32 GetHeight(BranchType* branch) noexcept
        {
            return branch == nullptr ? 0 : branch->Height;
        }

        static void UpdateHeight(BranchType* branch) noexcept
        {
            branch->Height = 1 + MC::Max(GetHeight(branch->LeftPart), GetHeight(branch->RightPart));
        }

        BranchType* RotateLeft(BranchType* branch) noexcept
        {
            BranchType* newHead = branch->RightPart;
            BranchType* middleBranch = newHead->LeftPart;

            Transplant(branch, newHead);

            newHead->LeftPart = branch;
            branch->Parent = newHead;

            branch->RightPart = middleBranch;
            if (middleBranch) middleBranch->Parent = branch;

            UpdateHeight(branch);
            UpdateHeight(newHead);

            return newHead;
        }

        BranchType* RotateRight(BranchType* branch) noexcept
        {
            BranchType* newHead = branch->LeftPart;
            BranchType* middleBranch = newHead->RightPart;

            Transplant(branch, newHead);

            newHead->RightPart = branch;
            branch->Parent = newHead;

            branch->LeftPart = middleBranch;
            if (middleBranch) middleBranch->Parent = branch;

            UpdateHeight(branch);
            UpdateHeight(newHead);

            return newHead;
        }

        BranchType* Rebalance(BranchType* branch) noexcept
        {
            UpdateHeight(branch);
            i32 balanceFactor = GetBalanceFactor(branch);

            if (balanceFactor > 1)
            {
                if (GetBalanceFactor(branch->LeftPart) < 0) RotateLeft(branch->LeftPart);
                return RotateRight(branch);
            }
            if (balanceFactor < -1)
            {
                if (GetBalanceFactor(branch->RightPart) > 0) RotateRight(branch->RightPart);
                return RotateLeft(branch);
            }
            return branch;
        }

        void RebalanceUpwards(BranchType* branch) noexcept
        {
            while (branch)
            {
                BranchType* parent = branch->Parent;
                u32 oldHeight = branch->Height;

                branch = Rebalance(branch);
                if (branch->Height == oldHeight) return;

                branch = parent;
            }
        }

        BranchType* AllocateBranch(StoredType&& value) noexcept
        {
            BranchType* branch = m_Allocator.Allocate(1);
            branch->Height = 1;
            branch->Parent = nullptr;
            branch->LeftPart = nullptr;
            branch->RightPart = nullptr;

            Memory::Construct(&branch->Value, Forward<StoredType>(value));
            return branch;
        }

        void DeallocateBranch(BranchType* branch) noexcept
        {
            Memory::Destroy(&branch->Value);
            m_Allocator.Deallocate(branch, 1);
        }

        BranchType* SearchBranch(const KeyType& key) const noexcept
        {
            BranchType* branch = m_Root;
            while (branch)
            {
                if (m_Compare(key, KeyOf(branch)))
                    branch = branch->LeftPart;
                else if (m_Compare(KeyOf(branch), key))
                    branch = branch->RightPart;
                else
                    return branch;
            }
            return nullptr;
        }

        static BranchType* GetLeftmostBranch(BranchType* branch) noexcept
        {
            if (!branch) return nullptr;
            while (branch->LeftPart != nullptr)
                branch = branch->LeftPart;
            return branch;
        }

        static BranchType* GetRightmostBranch(BranchType* branch) noexcept
        {
            if (!branch) return nullptr;
            while (branch->RightPart != nullptr)
                branch = branch->RightPart;
            return branch;
        }

        BranchType* CloneBranch(BranchType* other) noexcept
        {
            if (!other) return nullptr;
            BranchType* branch = AllocateBranch(StoredType(other->Value));
            branch->Height = other->Height;
            if (other->LeftPart != nullptr)
            {
                branch->LeftPart = CloneBranch(other->LeftPart);
                branch->LeftPart->Parent = branch;
            }
            if (other->RightPart != nullptr)
            {
                branch->RightPart = CloneBranch(other->RightPart);
                branch->RightPart->Parent = branch;
            }
            return branch;
        }

    private:
        BranchType* m_Root;
        usize m_ElementCount;

    private:
        AllocatorType m_Allocator;
        CompareFunctor m_Compare;
    };
}