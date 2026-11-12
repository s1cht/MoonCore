#pragma once
#include "Shared/TreeShared.hpp"

namespace MC
{
    template <Concept::TreeType Key,
        Concept::HeapAllocatorType Allocator = Memory::HeapAllocator<TreeBranch<Key>>,
        Concept::CompareFunctor Compare = Callables::Less<Key>>
    using Set = AVLTree<Key, void, Allocator, Compare, false>;
}