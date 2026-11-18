# MoonCore Containers API Reference {#mainpage}

Welcome to the **MoonCore Containers** technical documentation. MoonCore is a high-performance C++23 foundation library engineered for game engines, simulation systems, and performance-critical systems software.

---

## Architecture & Design Principles

The MoonCore containers library provides standard-compatible yet heavily optimized data structures tailored for predictability, control, and zero unnecessary overhead:

1. **C++23 Concepts**: All containers utilize modern C++23 concepts (`Concept::HeapAllocatorType`, `Concept::Character`, `Concept::CompareFunctor`, `Concept::TreeType`) to enforce strict interface contracts with clear compiler diagnostics.
2. **Explicit Memory Control**: Custom memory allocators (such as `MC::Memory::HeapAllocator`) are first-class citizens. Allocator instances are configurable and propagate cleanly across moves and copies.
3. **Exception-Free (`noexcept`) Architecture**: MoonCore containers are designed with noexcept guarantees. Validation and bounds verification utilize configurable debug assertion macros (`MOON_CHECK`, `MOON_VALIDATE`).
4. **Cache Locality & Compact Layouts**: Sequential memory structures (`MC::Array`, `MC::StackArray`, `MC::FixedArray`, `MC::Deque`) prioritize cache line alignment, contiguous backing stores, and minimal pointer chasing.

---

## Containers Overview & Classification

MoonCore containers are organized into five primary architectural categories:

### 1. Sequence Containers
| Container | Description | Storage Strategy |
| :--- | :--- | :--- |
| @ref MC::Array "MC::Array<T, Allocator>" | Dynamically resizable array with geometric capacity growth. | Contiguous heap allocation |
| @ref MC::FixedArray "MC::FixedArray<T, Capacity>" | Constant-size array with compile-time bounded capacity. | Contiguous inline/stack storage |
| @ref MC::StackArray "MC::StackArray<T, Capacity>" | Dynamic-size array with compile-time fixed maximum capacity. | In-place stack storage, no dynamic allocations |
| @ref MC::Deque "MC::Deque<T, BlockSize, Allocator>" | Double-ended queue with indexed block chunks. | Segmented blocks for efficient front/back insertion |

### 2. Associative Tree Containers
| Container | Description | Implementation |
| :--- | :--- | :--- |
| @ref MC::Map "MC::Map<Key, Value, Allocator, Compare>" | Ordered key-value dictionary with unique keys. | Self-balancing AVL tree (`MC::AVLTree`) |
| @ref MC::Set "MC::Set<Key, Allocator, Compare>" | Ordered set of unique keys. | Self-balancing AVL tree (`MC::AVLTree`) |
| @ref MC::Multimap "MC::Multimap<Key, Value, Allocator, Compare>" | Ordered key-value dictionary allowing duplicate keys. | Self-balancing AVL tree (`MC::AVLTree`) |
| @ref MC::Multiset "MC::Multiset<Key, Allocator, Compare>" | Ordered set allowing duplicate keys. | Self-balancing AVL tree (`MC::AVLTree`) |

### 3. Hash-Based Unordered Containers
| Container | Description | Lookup Complexity |
| :--- | :--- | :--- |
| @ref MC::UnorderedMap "MC::UnorderedMap<Key, Value, Hasher, Allocator>" | Hash table key-value map with bucket chaining. | Average O(1), Worst-case O(N) |
| @ref MC::UnorderedSet "MC::UnorderedSet<Key, Hasher, Allocator>" | Hash table set with bucket chaining. | Average O(1), Worst-case O(N) |

### 4. Container Adapters & Heaps
| Container | Description | Underlying Storage |
| :--- | :--- | :--- |
| @ref MC::Stack "MC::Stack<T, Container>" | LIFO (Last-In-First-Out) stack adapter. | Default: `MC::Array<T>` |
| @ref MC::Queue "MC::Queue<T, Container>" | FIFO (First-In-First-Out) queue adapter. | Default: `MC::Deque<T>` |
| @ref MC::BinaryHeap "MC::BinaryHeap<T, Compare, Container>" | Max/Min binary heap priority queue. | Default: `MC::Array<T>` |
| @ref MC::PriorityQueue "MC::PriorityQueue<T, Compare, Container, Heap>" | Strict priority queue adapter backed by a binary heap. | Default: `MC::BinaryHeap<T>` |

### 5. String & Bitwise Containers
| Container | Description | Characteristics |
| :--- | :--- | :--- |
| @ref MC::String "MC::String" / @ref MC::UTF8String "MC::UTF8String" | UTF-8 dynamic string with Small String Optimization (SSO). | Heap/SSO backing, UTF-8 codepoint iterations |
| @ref MC::WideString "MC::WideString" | UTF-16/Wide dynamic string. | Wide character dynamic storage |
| @ref MC::StringView "MC::StringView" | Non-owning, lightweight UTF-8 string slice. | Zero-allocation pointer + length view |
| @ref MC::WideStringView "MC::WideStringView" | Non-owning, lightweight UTF-16 string slice. | Zero-allocation pointer + length view |
| @ref MC::Bitset "MC::Bitset<BitCount>" | Compact fixed-size sequence of N bits packed into machine words. | 1 bit per boolean, bitwise vectorization |

---

## Complexity Matrix

| Container | Random Access | Front Insert / Erase | Back Insert / Erase | Search |
| :--- | :---: | :---: | :---: | :---: |
| **Array** | O(1) | O(N) | Amortized O(1) | O(N) |
| **FixedArray** | O(1) | N/A | N/A | O(N) |
| **StackArray** | O(1) | O(N) | O(1) (no alloc) | O(N) |
| **Deque** | O(1) | O(1) | O(1) | O(N) |
| **AVL Tree (Map / Set)** | N/A | O(log N) | O(log N) | O(log N) |
| **UnorderedMap / Set** | N/A | O(1) avg | O(1) avg | O(1) avg |
| **BinaryHeap** | N/A | N/A | O(log N) insert / O(1) top | O(N) |
| **Bitset** | O(1) | N/A | N/A | O(N) |

---

## Quick Start Examples

### Sequential Storage with `MC::Array`
```cpp
#include <MoonCore/Containers/Array.hpp>

MC::Array<int> numbers;
numbers.Reserve(64);

numbers.PushBack(10);
numbers.PushBack(20);
numbers.EmplaceBack(30);

for (int n : numbers)
{
    // Access elements sequentially
}
```

### Key-Value Storage with `MC::UnorderedMap`
```cpp
#include <MoonCore/Containers/UnorderedMap.hpp>
#include <MoonCore/Containers/String.hpp>

MC::UnorderedMap<MC::String, int> scores;
scores.Insert("Alice", 100);
scores.Insert("Bob", 85);

if (scores.Contains("Alice"))
{
    int score = scores["Alice"];
}
```

### Self-Balancing Ordered Storage with `MC::Map`
```cpp
#include <MoonCore/Containers/Map.hpp>

MC::Map<int, MC::String> orderedItems;
orderedItems.Insert(3, "Item 3");
orderedItems.Insert(1, "Item 1");
orderedItems.Insert(2, "Item 2");

for (auto it = orderedItems.Begin(); it != orderedItems.End(); ++it)
{
    // Iterates in strict key-sorted order: 1, 2, 3
}
```

---

## Building the Documentation

To build this documentation locally using Doxygen:

```bash
# From the repository root
doxygen Doxyfile
```

The resulting HTML documentation will be generated in `docs/html/index.html`. You can open it in any modern web browser.
