#pragma once
#include "MoonCore/Containers/Details/TreeShared.hpp"

namespace MC
{
    /**
     * @brief Ordered associative container containing a sorted set of unique objects of type @p Key.
     *
     * `Set<Key, Allocator, Compare>` is implemented as a self-balancing AVL binary search tree
     * (`MC::AVLTree`), guaranteeing logarithmic O(log N) search, insertion, and removal times.
     * Elements are stored and iterated in strictly ascending key order based on @p Compare.
     *
     * @tparam Key The key type. Must satisfy `Concept::TreeType`.
     * @tparam Allocator Memory allocator for tree branches. Defaults to `Memory::HeapAllocator<TreeBranch<Key>>`.
     * @tparam Compare Strict weak ordering comparison functor. Defaults to `Callables::Less<Key>`.
     *
     * @code
     * MC::Set<int> uniqueNumbers;
     * uniqueNumbers.Insert(42);
     * uniqueNumbers.Insert(17);
     * uniqueNumbers.Insert(42); // Not inserted (unique keys)
     *
     * for (int n : uniqueNumbers) {
     *     // Iterates in sorted order: 17, 42
     * }
     * @endcode
     */
    template <Concept::TreeType Key,
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<TreeBranch<Key>>,
        Concept::CompareFunctor Compare = Callables::Less<Key>>
    using Set = AVLTree<Key, void, Allocator, Compare, false>;
}
