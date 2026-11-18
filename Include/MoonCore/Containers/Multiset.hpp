#pragma once
#include "MoonCore/Containers/Details/TreeShared.hpp"

namespace MC
{
    /**
     * @brief Ordered associative container containing a sorted set of objects of type @p Key, allowing duplicate keys.
     *
     * `Multiset<Key, Allocator, Compare>` is implemented as a self-balancing AVL binary search tree
     * (`MC::AVLTree`), guaranteeing logarithmic O(log N) search, insertion, and deletion operations.
     *
     * @tparam Key The key type. Must satisfy `Concept::TreeType`.
     * @tparam Allocator Memory allocator for tree branches. Defaults to `Memory::HeapAllocator<TreeBranch<Key>>`.
     * @tparam Compare Strict weak ordering comparison functor. Defaults to `Callables::Less<Key>`.
     *
     * @code
     * MC::Multiset<int> bag;
     * bag.Insert(10);
     * bag.Insert(10);
     * bag.Insert(20);
     * @endcode
     */
    template <Concept::TreeType Key,
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<TreeBranch<Key>>,
        Concept::CompareFunctor Compare = Callables::Less<Key>>
    using Multiset = AVLTree<Key, void, Allocator, Compare, true>;
}
