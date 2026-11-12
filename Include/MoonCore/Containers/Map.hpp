#pragma once
#include "Details/TreeShared.hpp"

namespace MC
{
    template <Concept::TreeType Key, Concept::TreeType Value,
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<TreeBranch<Utility::Pair<const Key, Value>>>,
        Concept::CompareFunctor Compare = Callables::Less<Key>>
    using Map = AVLTree<Key, Value, Allocator, Compare, false>;
}