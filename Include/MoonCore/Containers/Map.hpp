#pragma once
#include "MoonCore/Containers/Details/TreeShared.hpp"

/**
 * @file Map.hpp
 * @brief Ordered associative container mapping unique keys to values.
 * @ingroup assoc_containers
 */

namespace MC
{
    /**
     * @brief Ordered associative container that stores key-value pairs with unique keys.
     *
     * `Map<Key, Value, Allocator, Compare>` is implemented as a self-balancing AVL binary search tree
     * (`MC::AVLTree`), guaranteeing O(log N) search, insertion, and removal times.
     * Elements are stored and iterated in strictly ascending key order based on @p Compare.
     *
     * @tparam Key The key type. Must satisfy `Concept::TreeType` (default and nothrow move constructible).
     * @tparam Value The mapped value type. Must satisfy `Concept::TreeType`.
     * @tparam Allocator Memory allocator for tree branches. Defaults to `Memory::HeapAllocator<TreeBranch<Pair<const Key, Value>>>`.
     * @tparam Compare Strict weak ordering comparison functor. Defaults to `Callables::Less<Key>`.
     *
     * @code
     * MC::Map<int, MC::String> dict;
     * dict.Insert(1, "One");
     * dict.Insert(2, "Two");
     * dict[3] = "Three";
     *
     * if (dict.Contains(2)) {
     *     auto& val = dict[2]; // "Two"
     * }
     * @endcode
     */
    template <Concept::TreeType Key, Concept::TreeType Value,
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<TreeBranch<Utility::Pair<const Key, Value>>>,
        Concept::CompareFunctor Compare = Callables::Less<Key>>
    using Map = AVLTree<Key, Value, Allocator, Compare, false>;
}
