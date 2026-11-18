#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Callables/Functors.hpp"
#include "MoonCore/Utility/Pair.hpp"

/**
 * @file TreeShared.hpp
 * @brief Self-balancing AVL binary search tree implementation backing ordered associative containers.
 * @ingroup assoc_containers
 */

namespace MC::Concept
{
    /** @brief Concept requiring default constructibility and nothrow move constructibility for tree keys and values. */
    template<typename T> concept TreeType = IsDefaultConstructible<T> && IsNothrowMoveConstructible<T>;
    /** @brief Concept allowing either a `TreeType` or `void` (for set containers). */
    template<typename T> concept TreeMappedType = TreeType<T> || IsVoid<T>;
}

namespace MC
{
    /**
     * @struct TreeTraits
     * @brief Type traits differentiating Map storage (`Pair<const Key, Value>`) from Set storage (`Key`).
     * @tparam Key Key type.
     * @tparam Value Mapped value type.
     */
    template<typename Key, typename Value>
    struct TreeTraits
    {
        using StoredType = Utility::Pair<const Key, Value>; ///< Internal node stored type.
        using ValueType = StoredType;                       ///< Exposed element value type.
        static constexpr bool IsSet = false;               ///< False for key-value maps.

        /** @brief Extracts const reference to key from stored pair. */
        static const Key& KeyOf(const StoredType& value) noexcept { return value.Key; }
    };

    /**
     * @struct TreeTraits<Key, void>
     * @brief Specialization for sets where only the key is stored.
     * @tparam Key Stored set key.
     */
    template<typename Key>
    struct TreeTraits<Key, void>
    {
        using StoredType = Key;                             ///< Stored element type.
        using ValueType = const Key;                        ///< Exposed element value type (immutable).
        static constexpr bool IsSet = true;                ///< True for sets.

        /** @brief Extracts const reference to key. */
        static const Key& KeyOf(const StoredType& value) noexcept { return value; }
    };

    /**
     * @struct TreeBranch
     * @brief Single node in the self-balancing AVL binary search tree.
     * @tparam ValueType Type of payload stored in the branch.
     */
    template <typename ValueType>
    struct TreeBranch
    {
        ValueType Value;             ///< Node value payload.
        u32 Height;                  ///< Subtree height for AVL balancing.
        TreeBranch* Parent;          ///< Pointer to parent node.
        TreeBranch* LeftPart;        ///< Pointer to left child subtree.
        TreeBranch* RightPart;       ///< Pointer to right child subtree.

        /**
         * @brief In-order successor traversal.
         * @return Pointer to next higher node, or nullptr if this is the maximum.
         */
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

        /**
         * @brief In-order successor traversal (const overload).
         * @return Const pointer to next higher node, or nullptr if maximum.
         */
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

        /**
         * @brief In-order predecessor traversal.
         * @return Pointer to previous lower node, or nullptr if minimum.
         */
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

        /**
         * @brief In-order predecessor traversal (const overload).
         * @return Const pointer to previous lower node, or nullptr if minimum.
         */
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

    /**
     * @class TreeIterator
     * @brief Bidirectional iterator for traversing AVL tree nodes in sorted order.
     * @tparam Container Owning tree container type.
     * @tparam IsConst Whether this iterator provides const access.
     */
    template<typename Container, bool IsConst = false>
    class TreeIterator
    {
    public:
        using KeyType = typename Container::KeyType;                   ///< Key type.
        using ValueType = typename Container::ValueType;               ///< Value payload type.
        using DiffType = iptr;                                         ///< Distance difference type.
        using PtrType = Conditional<IsConst, const ValueType*, ValueType*>; ///< Pointer to value.
        using RefType = Conditional<IsConst, const ValueType&, ValueType&>; ///< Reference to value.
        using Category = Iterator::Tags::Bidirectional;                ///< Bidirectional iterator tag.
        using BranchPtrType = typename Container::BranchType*;         ///< Pointer to tree node.

    public:
        /** @brief Default constructor. Constructs a null iterator (equivalent to `End()`). */
        constexpr TreeIterator() noexcept : m_Ptr(nullptr) {}
        /** @brief Constructs an iterator pointing to a specific tree branch node. */
        constexpr TreeIterator(BranchPtrType ptr) noexcept : m_Ptr(ptr) {}

        /** @brief Copy constructor. */
        constexpr TreeIterator(const TreeIterator& other) noexcept { m_Ptr = other.m_Ptr; }
        /** @brief Move constructor. */
        constexpr TreeIterator(TreeIterator&& other) noexcept { m_Ptr = other.m_Ptr; }

    public:
        /** @brief Copy assignment operator. */
        constexpr TreeIterator& operator=(const TreeIterator& other) noexcept { m_Ptr = other.m_Ptr; return *this; }
        /** @brief Move assignment operator. */
        constexpr TreeIterator& operator=(TreeIterator&& other) noexcept { m_Ptr = Move(other.m_Ptr); return *this; }

    public:
        /** @brief Advances iterator by @p index steps using in-order successor traversal. */
        constexpr TreeIterator& operator+=(usize index) { for (usize i = 0; i < index; i++) m_Ptr = m_Ptr->Successor(); return *this; }
        /** @brief Decrements iterator by @p index steps using in-order predecessor traversal. */
        constexpr TreeIterator& operator-=(usize index) { for (usize i = 0; i < index; i++) m_Ptr = m_Ptr->Precessor(); return *this; }

        /** @brief Returns an iterator advanced by @p index positions. */
        constexpr TreeIterator operator+(usize index) const { return TreeIterator(m_Ptr) += index; }
        /** @brief Returns an iterator moved back by @p index positions. */
        constexpr TreeIterator operator-(usize index) const { return TreeIterator(m_Ptr) -= index; }

        /** @brief Pre-increment operator. Advances to in-order successor. */
        constexpr TreeIterator& operator++() { m_Ptr = m_Ptr->Successor(); return *this; }
        /** @brief Post-increment operator. */
        constexpr TreeIterator operator++(int) { TreeIterator temp = *this; m_Ptr = m_Ptr->Successor(); return temp; }

        /** @brief Pre-decrement operator. Advances to in-order predecessor. */
        constexpr TreeIterator& operator--() { m_Ptr = m_Ptr->Precessor(); return *this; }
        /** @brief Post-decrement operator. */
        constexpr TreeIterator operator--(int) { TreeIterator temp = *this; m_Ptr = m_Ptr->Precessor(); return temp; }

        /** @brief Equality comparison. */
        constexpr bool operator==(const TreeIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
        /** @brief Inequality comparison. */
        constexpr bool operator!=(const TreeIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }

        /** @brief Subscript access relative to current position. */
        constexpr RefType operator[](usize index) const { return (*this + index).m_Ptr->Value; }
        /** @brief Member access operator. */
        constexpr PtrType operator->() const { return &m_Ptr->Value; }
        /** @brief Dereference operator. */
        constexpr RefType operator*() const { return m_Ptr->Value; }

    public:
        BranchPtrType m_Ptr; ///< Pointer to current tree node.
    };

    /**
     * @class AVLTree
     * @brief Self-balancing AVL binary search tree with strict weak ordering guarantees.
     *
     * `AVLTree<Key, Value, Allocator, Compare, AllowDuplicates>` is the underlying data structure
     * backing `MC::Map`, `MC::Set`, `MC::Multimap`, and `MC::Multiset`.
     *
     * In an AVL tree, the heights of two child subtrees of any node differ by at most one.
     * Lookup, insertion, and deletion all run in guaranteed O(log N) time in both average and worst cases.
     *
     * @tparam Key The key type used for sorting and lookup.
     * @tparam Value Mapped type, or `void` if functioning as a set.
     * @tparam Allocator Memory allocator for tree branches.
     * @tparam Compare Strict weak ordering comparison functor.
     * @tparam AllowDuplicates Whether equivalent keys are permitted (true for Multimap/Multiset).
     */
    template <typename Key, Concept::TreeMappedType Value,
        Concept::HeapAllocatorType Allocator,
        Concept::CompareFunctor Compare,
        bool AllowDuplicates>
    class AVLTree
    {
    public:
        using TraitsType = TreeTraits<Key, Value>;                      ///< Internal trait mapping.

        using KeyType = Key;                                           ///< Key type.
        using MappedType = Value;                                       ///< Mapped value type.
        using StoredType = typename TraitsType::StoredType;            ///< Internal payload type stored in node.
        using ValueType = typename TraitsType::ValueType;              ///< Exposed element value type.
        using BranchType = TreeBranch<StoredType>;                     ///< Node branch type.
        using PtrType = ValueType*;                                    ///< Pointer to element.
        using ConstPtrType = const ValueType*;                          ///< Pointer to const element.
        using RefType = ValueType&;                                    ///< Reference to element.
        using ConstRefType = const ValueType&;                          ///< Reference to const element.

        static constexpr bool IsSet = TraitsType::IsSet;               ///< True if container is a Set / Multiset.
        static constexpr bool IsMulti = AllowDuplicates;               ///< True if duplicate keys are allowed.

    public:
        using AllocatorType = Allocator;                               ///< Node allocator policy.
        using CompareFunctor = Compare;                                 ///< Comparator functor type.

        using Iterator = TreeIterator<AVLTree, false>;                  ///< Bidirectional iterator.
        using ConstIterator = TreeIterator<AVLTree, true>;              ///< Const bidirectional iterator.
        using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;///< Reverse iterator.
        using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>; ///< Const reverse iterator.

    public:
        /** @brief Default constructor. Constructs an empty tree. */
        constexpr AVLTree() noexcept
            : m_Root(nullptr), m_ElementCount(0), m_Allocator(AllocatorType())
        {}

        /**
         * @brief Copy constructor. Deep copies all nodes from @p other.
         * @param other Source tree.
         */
        AVLTree(const AVLTree& other) noexcept
        {
            m_ElementCount = other.m_ElementCount;
            m_Allocator = other.m_Allocator;
            m_Root = CloneBranch(other.m_Root);
        }

        /**
         * @brief Move constructor. Transfers root pointer and clears @p other.
         * @param other Source tree.
         */
        AVLTree(AVLTree&& other) noexcept
        {
            m_Allocator = other.m_Allocator;
            m_ElementCount = other.m_ElementCount;
            m_Root = other.m_Root;

            other.m_ElementCount = 0;
            other.m_Root = nullptr;
        }

        /** @brief Destructor. Destroys all nodes and frees memory. */
        ~AVLTree() noexcept
        {
            m_ElementCount = 0;
            Clear();
        }

    public:
        /** @brief Returns number of elements in tree. */
        usize Size() const noexcept { return m_ElementCount; }
        /** @brief Returns pointer to root branch. */
        const BranchType* Data() const noexcept { return m_Root; }
        /** @brief Checks whether tree is empty. */
        bool Empty() const noexcept { return m_Root == nullptr; }

        /** @brief Accesses minimum element (leftmost node). */
        ValueType& Front() const noexcept { return GetLeftmostBranch(m_Root)->Value; }
        /** @brief Accesses maximum element (rightmost node). */
        ValueType& Back() const noexcept { return GetRightmostBranch(m_Root)->Value; }

    public:
        /** @brief Returns iterator to minimum element in tree. */
        Iterator Begin() noexcept { return Iterator(GetLeftmostBranch(m_Root)); }
        /** @brief Returns const iterator to minimum element in tree. */
        ConstIterator Begin() const noexcept { return ConstIterator(GetLeftmostBranch(m_Root)); }
        /** @brief Returns const iterator to minimum element in tree. */
        ConstIterator CBegin() const noexcept { return ConstIterator(GetLeftmostBranch(m_Root)); }
        /** @brief Returns iterator to past-the-end. */
        Iterator End() noexcept { return Iterator(nullptr); }
        /** @brief Returns const iterator to past-the-end. */
        ConstIterator End() const noexcept { return ConstIterator(nullptr); }
        /** @brief Returns static const iterator to past-the-end. */
        static ConstIterator CEnd() noexcept { return ConstIterator(nullptr); }

        /** @brief Returns reverse iterator to maximum element. */
        ReverseIterator RBegin() noexcept { return m_Root != nullptr ? ReverseIterator(GetRightmostBranch(m_Root)) : REnd(); }
        /** @brief Returns const reverse iterator to maximum element. */
        ConstReverseIterator RBegin() const noexcept { return m_Root != nullptr ? ConstReverseIterator(GetRightmostBranch(m_Root)) : REnd(); }
        /** @brief Returns const reverse iterator to maximum element. */
        ConstReverseIterator CRBegin() const noexcept { return m_Root != nullptr ? ConstReverseIterator(GetRightmostBranch(m_Root)) : CREnd(); }
        /** @brief Returns reverse iterator to past-the-beginning. */
        ReverseIterator REnd() noexcept { return ReverseIterator(nullptr); }
        /** @brief Returns const reverse iterator to past-the-beginning. */
        ConstReverseIterator REnd() const noexcept { return ConstReverseIterator(nullptr); }
        /** @brief Returns static const reverse iterator to past-the-beginning. */
        static ConstReverseIterator CREnd() noexcept { return ConstReverseIterator(nullptr); }

    public:
        /** @brief Range-for compatible begin iterator. */
        Iterator begin() noexcept { return Begin(); }
        /** @brief Range-for compatible const begin iterator. */
        ConstIterator begin() const noexcept { return Begin(); }
        /** @brief Range-for compatible end iterator. */
        Iterator end() noexcept { return End(); }
        /** @brief Range-for compatible const end iterator. */
        ConstIterator end() const noexcept { return End(); }

    public:
        /** @brief Copy assignment operator. */
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

        /** @brief Move assignment operator. */
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

        /**
         * @brief Subscript access operator (Map only).
         *
         * If @p key exists, returns reference to mapped value. Otherwise, default-constructs
         * a new element with @p key and returns reference to newly created value.
         *
         * @tparam SameMappedType Internal type validation.
         * @param key Key to find or insert.
         * @return Reference to mapped value.
         */
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
        /**
         * @brief Inserts key-value pair into map by copying.
         * @param key Key to insert.
         * @param value Value to insert.
         * @return Pair containing iterator to element and boolean indicating if insertion took place.
         */
        template<typename ConvertibleMappedType = MappedType>
            requires IsConvertible<ConvertibleMappedType, MappedType>
        Utility::Pair<Iterator, bool> Insert(const KeyType& key, const ConvertibleMappedType& value) noexcept
        {
            static_assert(!IsSet, "Insert(Key, Value) is only accessible in Maps!");
            return EmplaceImpl(ValueType( key, static_cast<const MappedType&>(value) ));
        }

        /**
         * @brief Inserts key-value pair into map by moving.
         * @param key Key to insert.
         * @param value Value to insert.
         * @return Pair containing iterator to element and boolean indicating if insertion took place.
         */
        template<typename ConvertibleMappedType = MappedType>
            requires IsConvertible<ConvertibleMappedType, MappedType>
        Utility::Pair<Iterator, bool> Insert(KeyType&& key, ConvertibleMappedType&& value) noexcept
        {
            static_assert(!IsSet, "Insert(Key, Value) is only accessible in Maps!");
            return EmplaceImpl(StoredType(Move(key), Move(static_cast<MappedType&&>(value))));
        }

        /**
         * @brief Inserts stored element (value or key-value pair) by copying.
         * @param value Element to insert.
         * @return Pair containing iterator to element and boolean indicating if insertion took place.
         */
        Utility::Pair<Iterator, bool> Insert(const StoredType& value) noexcept { return EmplaceImpl(StoredType(value)); }

        /**
         * @brief Inserts stored element by moving.
         * @param value Element to insert.
         * @return Pair containing iterator to element and boolean indicating if insertion took place.
         */
        Utility::Pair<Iterator, bool> Insert(StoredType&& value) noexcept { return EmplaceImpl(Forward<StoredType>(value)); }

        /**
         * @brief Constructs element in-place in tree.
         * @tparam ArgTypes Constructor arguments.
         * @param args Forwarded arguments.
         * @return Pair containing iterator to element and boolean indicating if insertion took place.
         */
        template<typename... ArgTypes>
        Utility::Pair<Iterator, bool> Emplace(ArgTypes&&... args) noexcept
        {
            return EmplaceOrAssignImpl(StoredType{ Forward<ArgTypes>(args)... });
        }

        /**
         * @brief Erases element matching @p key.
         * @param key Key to remove.
         * @return For single-key trees, iterator following removed node; for multi-key trees, number of removed elements.
         */
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

        /** @brief Erases node pointed to by @p iterator. */
        Iterator Erase(Iterator iterator) noexcept { return EraseImpl(iterator.m_Ptr); }
        /** @brief Erases node pointed to by @p iterator (const iterator overload). */
        Iterator Erase(ConstIterator iterator) noexcept { return EraseImpl(iterator.m_Ptr); }
        /** @brief Erases nodes in range `[first, last)`. */
        Iterator Erase(Iterator first, Iterator last) noexcept { while (first != last) first = EraseImpl(first.m_Ptr); return last; }
        /** @brief Erases nodes in range `[first, last)` (const overload). */
        Iterator Erase(ConstIterator first, ConstIterator last) noexcept
        {
            while (first != last) first = ConstIterator(EraseImpl(first.m_Ptr).m_Ptr);
            return Iterator(last.m_Ptr);
        }

        /**
         * @brief Checks whether tree contains a node with the given @p key.
         * @param key Key to search for.
         * @return True if key is found, false otherwise.
         * @note O(log N) time complexity.
         */
        bool Contains(const KeyType& key) const noexcept { return FindBranch(key) != nullptr; }

        /**
         * @brief Accesses mapped value with bounds checking (Map only).
         * @param key Key to lookup.
         * @return Mutable reference to value.
         */
        template<typename SameMappedType = MappedType>
            requires IsSame<SameMappedType, MappedType>
        SameMappedType& At(const KeyType& key) noexcept
        {
            static_assert(!AllowDuplicates && !IsSet, "At() is only accessible in Map!");
            BranchType* node = FindBranch(key);
            MOON_VALIDATE(node, "Key doesn't exits in map");
            return node->Value.Value;
        }

        /**
         * @brief Accesses mapped value with bounds checking (Map only, const overload).
         * @param key Key to lookup.
         * @return Const reference to value.
         */
        template<typename SameMappedType = MappedType>
            requires IsSame<SameMappedType, MappedType>
        const SameMappedType& At(const KeyType& key) const noexcept
        {
            static_assert(!AllowDuplicates && !IsSet, "At() is only accessible in Map!");
            const BranchType* node = FindBranch(key);
            MOON_VALIDATE(node, "Key doesn't exits in map");
            return node->Value.Value;
        }

        /** @brief Destroys all nodes and clears the tree. */
        void Clear() noexcept
        {
            ClearImpl();
        }

        /**
         * @brief Finds node matching @p key.
         * @param key Key to find.
         * @return Iterator to found node, or `End()` if not found.
         * @note O(log N) time complexity.
         */
        Iterator Find(const KeyType& key) noexcept { return Iterator(FindBranch(key)); }

        /**
         * @brief Finds node matching @p key (const overload).
         * @param key Key to find.
         * @return Const iterator to found node, or `End()` if not found.
         * @note O(log N) time complexity.
         */
        ConstIterator Find(const KeyType& key) const noexcept { return ConstIterator(FindBranch(key)); }

        /**
         * @brief Returns an iterator to the first element not less than @p key.
         * @param key Key value to compare.
         * @return Iterator to lower bound, or `End()`.
         * @note O(log N) time complexity.
         */
        Iterator LowerBound(const KeyType& key) noexcept { return Iterator(LowerBoundImpl(key)); }

        /**
         * @brief Returns a const iterator to the first element not less than @p key.
         * @param key Key value to compare.
         * @return Const iterator to lower bound, or `End()`.
         * @note O(log N) time complexity.
         */
        ConstIterator LowerBound(const KeyType& key) const noexcept { return ConstIterator(LowerBoundImpl(key)); }

        /**
         * @brief Returns an iterator to the first element greater than @p key.
         * @param key Key value to compare.
         * @return Iterator to upper bound, or `End()`.
         * @note O(log N) time complexity.
         */
        Iterator UpperBound(const KeyType& key) noexcept { return Iterator(UpperBoundImpl(key)); }

        /**
         * @brief Returns a const iterator to the first element greater than @p key.
         * @param key Key value to compare.
         * @return Const iterator to upper bound, or `End()`.
         * @note O(log N) time complexity.
         */
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
                        currentBranch->Value = Forward<StoredType>(value);
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

        BranchType* EraseImpl(BranchType* branch) noexcept
        {
            BranchType* nextNode = branch->Successor();
            BranchType* rebalanceStart = nullptr;

            if (branch->LeftPart && branch->RightPart)
            {
                BranchType* successor = branch->RightPart;
                while (successor->LeftPart) successor = successor->LeftPart;

                rebalanceStart = (successor->Parent == branch) ? successor : successor->Parent;

                BranchType* succRight = successor->RightPart;
                BranchType* succParent = successor->Parent;

                if (succParent != branch)
                {
                    succParent->LeftPart = succRight;
                    if (succRight) succRight->Parent = succParent;

                    successor->RightPart = branch->RightPart;
                    branch->RightPart->Parent = successor;
                }

                successor->LeftPart = branch->LeftPart;
                branch->LeftPart->Parent = successor;

                successor->Parent = branch->Parent;
                if (!branch->Parent)
                    m_Root = successor;
                else if (branch->Parent->LeftPart == branch)
                    branch->Parent->LeftPart = successor;
                else
                    branch->Parent->RightPart = successor;

                successor->Height = branch->Height;
            }
            else
            {
                BranchType* child = branch->LeftPart ? branch->LeftPart : branch->RightPart;
                rebalanceStart = branch->Parent;

                if (child) child->Parent = branch->Parent;

                if (!branch->Parent)
                    m_Root = child;
                else if (branch->Parent->LeftPart == branch)
                    branch->Parent->LeftPart = child;
                else
                    branch->Parent->RightPart = child;
            }

            DeallocateBranch(branch);
            --m_ElementCount;

            if (rebalanceStart)
                RebalanceUpwards(rebalanceStart);

            return nextNode;
        }

        BranchType* AllocateBranch(StoredType&& value) noexcept
        {
            BranchType* branch = m_Allocator.Allocate(1);
            branch->Value = Forward<StoredType>(value);
            branch->Height = 1;
            branch->Parent = nullptr;
            branch->LeftPart = nullptr;
            branch->RightPart = nullptr;
            return branch;
        }

        void DeallocateBranch(BranchType* branch) noexcept
        {
            Memory::Destroy(&branch->Value);
            m_Allocator.Deallocate(branch, 1);
        }

        void ClearImpl() noexcept
        {
            if (!m_Root) return;
            DestroyBranch(m_Root);
            m_Root = nullptr;
            m_ElementCount = 0;
        }

        void DestroyBranch(BranchType* branch) noexcept
        {
            if (!branch) return;
            DestroyBranch(branch->LeftPart);
            DestroyBranch(branch->RightPart);
            DeallocateBranch(branch);
        }

        BranchType* CloneBranch(const BranchType* other) noexcept
        {
            if (!other) return nullptr;
            BranchType* branch = m_Allocator.Allocate(1);
            branch->Value = other->Value;
            branch->Height = other->Height;
            branch->Parent = nullptr;
            branch->LeftPart = CloneBranch(other->LeftPart);
            branch->RightPart = CloneBranch(other->RightPart);
            if (branch->LeftPart) branch->LeftPart->Parent = branch;
            if (branch->RightPart) branch->RightPart->Parent = branch;
            return branch;
        }

        static usize GetHeight(const BranchType* branch) noexcept { return branch ? branch->Height : 0; }
        static isize GetBalanceFactor(const BranchType* branch) noexcept { return branch ? static_cast<isize>(GetHeight(branch->LeftPart)) - static_cast<isize>(GetHeight(branch->RightPart)) : 0; }

        static void UpdateHeight(BranchType* branch) noexcept
        {
            branch->Height = 1 + Max(GetHeight(branch->LeftPart), GetHeight(branch->RightPart));
        }

        BranchType* RotateRight(BranchType* y) noexcept
        {
            BranchType* x = y->LeftPart;
            BranchType* T2 = x->RightPart;

            x->RightPart = y;
            y->LeftPart = T2;

            if (T2) T2->Parent = y;
            x->Parent = y->Parent;
            y->Parent = x;

            UpdateHeight(y);
            UpdateHeight(x);

            return x;
        }

        BranchType* RotateLeft(BranchType* x) noexcept
        {
            BranchType* y = x->RightPart;
            BranchType* T2 = y->LeftPart;

            y->LeftPart = x;
            x->RightPart = T2;

            if (T2) T2->Parent = x;
            y->Parent = x->Parent;
            x->Parent = y;

            UpdateHeight(x);
            UpdateHeight(y);

            return y;
        }

        void RebalanceUpwards(BranchType* node) noexcept
        {
            while (node)
            {
                UpdateHeight(node);
                const isize balance = GetBalanceFactor(node);

                BranchType* parent = node->Parent;
                BranchType* newSubtreeRoot = node;

                if (balance > 1)
                {
                    if (GetBalanceFactor(node->LeftPart) < 0)
                    {
                        node->LeftPart = RotateLeft(node->LeftPart);
                    }
                    newSubtreeRoot = RotateRight(node);
                }
                else if (balance < -1)
                {
                    if (GetBalanceFactor(node->RightPart) > 0)
                    {
                        node->RightPart = RotateRight(node->RightPart);
                    }
                    newSubtreeRoot = RotateLeft(node);
                }

                if (newSubtreeRoot != node)
                {
                    newSubtreeRoot->Parent = parent;
                    if (!parent)
                        m_Root = newSubtreeRoot;
                    else if (parent->LeftPart == node)
                        parent->LeftPart = newSubtreeRoot;
                    else
                        parent->RightPart = newSubtreeRoot;
                }

                node = parent;
            }
        }

        static BranchType* GetLeftmostBranch(BranchType* branch) noexcept
        {
            if (!branch) return nullptr;
            while (branch->LeftPart) branch = branch->LeftPart;
            return branch;
        }

        static BranchType* GetRightmostBranch(BranchType* branch) noexcept
        {
            if (!branch) return nullptr;
            while (branch->RightPart) branch = branch->RightPart;
            return branch;
        }

        BranchType* SearchBranch(const KeyType& key) const noexcept
        {
            BranchType* current = m_Root;
            while (current)
            {
                if (m_Compare(key, KeyOf(current)))
                    current = current->LeftPart;
                else if (m_Compare(KeyOf(current), key))
                    current = current->RightPart;
                else
                    return current;
            }
            return nullptr;
        }

    private:
        BranchType* m_Root;            ///< Pointer to root node of AVL tree.
        usize m_ElementCount;          ///< Total number of nodes in tree.
        AllocatorType m_Allocator;     ///< Allocator policy for tree nodes.
        CompareFunctor m_Compare;       ///< Strict weak ordering comparison functor.
    };
}
