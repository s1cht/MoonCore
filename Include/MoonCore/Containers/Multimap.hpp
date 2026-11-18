#pragma once
#include "MoonCore/Containers/Details/TreeShared.hpp"

/**
 * @file Multimap.hpp
 * @brief Ordered associative container mapping keys to values, permitting duplicate keys.
 * @ingroup assoc_containers
 */

namespace MC
{
    /**
     * @brief Ordered associative container that stores key-value pairs while allowing duplicate keys.
     *
     * `Multimap<Key, Value, Allocator, Compare>` is implemented as a self-balancing AVL binary search tree
     * (`MC::AVLTree`), guaranteeing logarithmic O(log N) operations.
     * Elements with equivalent keys are stored together in sorted order.
     *
     * @tparam Key The key type. Must satisfy `Concept::TreeType`.
     * @tparam Value The mapped value type. Must satisfy `Concept::TreeType`.
     * @tparam Allocator Memory allocator for tree branches. Defaults to `Memory::HeapAllocator<TreeBranch<Pair<const Key, Value>>>`.
     * @tparam Compare Strict weak ordering comparison functor. Defaults to `Callables::Less<Key>`.
     *
     * @code
     * MC::Multimap<int, MC::String> multimap;
     * multimap.Insert(1, "Alpha");
     * multimap.Insert(1, "Beta");
     * multimap.Insert(2, "Gamma");
     * @endcode
     */
    template <Concept::TreeType Key, Concept::TreeType Value,
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<TreeBranch<Utility::Pair<const Key, Value>>>,
        Concept::CompareFunctor Compare = Callables::Less<Key>>
    using Multimap = AVLTree<Key, Value, Allocator, Compare, true>;
}
