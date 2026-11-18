#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Pair.hpp"
#include "MoonCore/Memory/Hasher.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Containers/String.hpp"

/**
 * @file UnorderedMap.hpp
 * @brief High-performance hash table key-value map container with bucket chaining.
 * @ingroup unord_containers
 */

namespace MC
{
	/**
	 * @class UMapIterator
	 * @brief Forward iterator for traversing key-value pairs in an `MC::UnorderedMap`.
	 *
	 * Iterates over non-empty hash buckets and follows bucket node chains.
	 *
	 * @tparam Container Owning unordered map container type.
	 * @tparam IsConst Whether this iterator provides const access to values.
	 */
	template <typename Container, bool IsConst = false>
	class UMapIterator
	{
	public:
		using KeyType		= typename Container::KeyType;       ///< Key type.
		using ValueType		= typename Container::ValueType;     ///< Value pair type.
		using DiffType		= iptr;                             ///< Difference type.
		using PtrType		= typename Container::NodeType*;    ///< Pointer to node.
		using ValuePtrType	= Conditional<IsConst, const ValueType*, ValueType*>; ///< Pointer to value.
		using RefType		= Conditional<IsConst, const ValueType&, ValueType&>; ///< Reference to value.
		using Category		= Iterator::Tags::Forward;          ///< Forward iterator tag.

	public:
		using key_type = KeyType;
		using value_type = ValueType;
		using difference_type = DiffType;
		using pointer = PtrType;
		using reference = RefType;
		using iterator_category = Category;

	public:
		/** @brief Default constructor. Constructs a null iterator. */
		constexpr UMapIterator() noexcept
		    : m_Ptr(nullptr), m_Buckets(nullptr), m_BucketCount(0), m_BucketIndex(0)
		{}

		/**
		 * @brief Constructs an iterator targeting a specific node and bucket index.
		 * @param ptr Pointer to current node.
		 * @param buckets Array of bucket head pointers.
		 * @param bucketCount Total number of buckets in table.
		 * @param bucketIndex Current bucket index.
		 */
		constexpr UMapIterator(PtrType ptr, PtrType* buckets, usize bucketCount, usize bucketIndex)
			: m_Ptr(ptr), m_Buckets(buckets), m_BucketCount(bucketCount), m_BucketIndex(bucketIndex)
		{}

	public:
		/** @brief Pre-increment operator. Advances to next element in chain or next non-empty bucket. */
		constexpr UMapIterator& operator++() noexcept
		{
			if (m_Ptr && m_Ptr->NextNode) m_Ptr = m_Ptr->NextNode;
			else
			{
				m_Ptr = nullptr;
				for (usize i = m_BucketIndex + 1; i < m_BucketCount; ++i)
				{
					if (m_Buckets[i])
					{
						m_Ptr = m_Buckets[i];
						m_BucketIndex = i;
						break;
					}
				}
			}
			return *this;
		}

		/** @brief Post-increment operator. */
		constexpr UMapIterator operator++(int) noexcept
		{
			UMapIterator temp = *this;
			++(*this);
			return temp;
		}

		/** @brief Equality comparison. */
		constexpr bool operator==(const UMapIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		/** @brief Inequality comparison. */
		constexpr bool operator!=(const UMapIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }

		/** @brief Member access operator. */
		constexpr ValuePtrType operator->() const noexcept { return &m_Ptr->Pair; }
		/** @brief Dereference operator. */
		constexpr RefType operator*() const noexcept { return m_Ptr->Pair; }

	private:
		PtrType m_Ptr;
		PtrType* m_Buckets;
		usize m_BucketCount;
		usize m_BucketIndex;

	private:
		friend Container;
	};

	namespace Helper
	{
		/**
		 * @struct MapNode
		 * @brief Singly linked bucket node for `MC::UnorderedMap`.
		 * @tparam KeyT Key type.
		 * @tparam ValueT Mapped value type.
		 */
		template <typename KeyT, typename ValueT>
		struct MapNode
		{
			using KeyType = KeyT;
			using ValueType = ValueT;
			using PairType = Utility::Pair<const KeyType, ValueType>;

		public:
			PairType Pair;      ///< Key-value pair payload.
			MapNode* NextNode;  ///< Next node in bucket collision chain.

			constexpr MapNode(const KeyType& k, const ValueType& v) noexcept
				: Pair({ k, v }), NextNode(nullptr)
			{}

			constexpr MapNode(KeyType&& k, ValueType&& v) noexcept
				: Pair({ Move(k), Move(v) }), NextNode(nullptr)
			{}

			constexpr const KeyType& Key() const noexcept { return Pair.Key; }
			constexpr ValueType& Value() noexcept { return Pair.Value; }
			constexpr PairType& ToPair() noexcept { return Pair; }
		};

		/**
		 * @struct UnorderedMapTraits
		 * @brief Internal type bundle for `MC::UnorderedMap`.
		 */
		template<typename Key, typename Value,
			template<typename> class Hasher = Memory::Hasher,
			template<typename> class Allocator = Memory::HeapAllocator>
	    struct UnorderedMapTraits
	    {
		public:
			using KeyType		= Key;
			using MappedType	= Value;
			using ValueType		= MC::Utility::Pair<const KeyType, MappedType>;
			using NodeType		= MapNode<KeyType, MappedType>;
			using PtrType		= NodeType*;
			using ConstPtrType	= const NodeType*;
			using RefType		= ValueType&;
			using ConstRefType	= const ValueType&;

		public:
			using HasherType		= Hasher<KeyType>;
			using NodeAllocator		= Allocator<NodeType>;
			using BucketAllocator	= Allocator<NodeType*>;
	    };
	}

	namespace Concept
	{
		/**
		 * @concept TemplateArgumentsEligibleForUnorderedMap
		 * @brief Validates template arguments for `MC::UnorderedMap`.
		 */
		template<typename Key, typename Value, template<typename> class Hasher, template<typename> class Allocator>
		concept TemplateArgumentsEligibleForUnorderedMap = 
			IsDefaultConstructible<Key> && IsDefaultConstructible<Value> &&
			IsNothrowCopyConstructible<Key> && IsNothrowCopyConstructible<Value> &&
			IsNothrowMoveConstructible<Key> && IsNothrowMoveConstructible<Value> &&
			Concept::HasherType<typename MC::Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>::HasherType>&&
			Concept::HeapAllocatorType<typename MC::Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>::NodeAllocator>&&
			Concept::HeapAllocatorType<typename MC::Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>::BucketAllocator>;
	}

	/**
	 * @class UnorderedMap
	 * @brief Hash table container with key-value pairs, unique keys, and separate chaining.
	 *
	 * `UnorderedMap<Key, Value, Hasher, Allocator>` organizes elements into buckets using a hash function.
	 * Provides average constant-time O(1) search, insertion, and deletion. When the load factor exceeds
	 * @p maxLoadFactor, the table rehashes automatically.
	 *
	 * @tparam Key Key type.
	 * @tparam Value Mapped value type.
	 * @tparam Hasher Hash functor template satisfying `Concept::HasherType`. Defaults to `Memory::Hasher`.
	 * @tparam Allocator Allocator policy template. Defaults to `Memory::HeapAllocator`.
	 *
	 * @code
	 * MC::UnorderedMap<int, MC::String> map;
	 * map.Insert(1, "Alpha");
	 * map.Insert(2, "Beta");
	 * map[3] = "Gamma";
	 *
	 * if (map.Contains(2)) {
	 *     MC::String val = map.At(2).Value;
	 * }
	 * @endcode
	 */
	template <typename Key, typename Value,
		template<typename> class Hasher = Memory::Hasher,
		template<typename> class Allocator = Memory::HeapAllocator>
		requires Concept::TemplateArgumentsEligibleForUnorderedMap<Key, Value, Hasher, Allocator>
    class UnorderedMap
	{
	    using Traits = Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>;

	public:
	    using KeyType	   = typename Traits::KeyType;         ///< Key type.
		using MappedType   = typename Traits::MappedType;      ///< Mapped value type.
		using ValueType	   = typename Traits::ValueType;       ///< Key-value pair type.
		using NodeType	   = typename Traits::NodeType;        ///< Internal node type.
		using PtrType	   = typename Traits::PtrType;         ///< Pointer to node.
		using ConstPtrType = typename Traits::ConstPtrType;    ///< Pointer to const node.
		using RefType	   = typename Traits::RefType;         ///< Reference to pair.
		using ConstRefType = typename Traits::ConstRefType;    ///< Reference to const pair.

	public:
		using HasherType	  = typename Traits::HasherType;      ///< Hasher type.
		using NodeAllocator	  = typename Traits::NodeAllocator;   ///< Node allocator type.
		using BucketAllocator = typename Traits::BucketAllocator; ///< Bucket array allocator type.
		
	public:
		using Iterator      = UMapIterator<UnorderedMap, false>; ///< Forward iterator.
		using ConstIterator = UMapIterator<UnorderedMap, true>;  ///< Const forward iterator.

	public:
		static constexpr usize REHASH_MULTIPLIER = 2;          ///< Bucket array growth factor.

	public:
		/**
		 * @brief Constructs an empty unordered map with initial bucket count and load factor.
		 * @param initialSize Initial number of buckets (default 16).
		 * @param maxLoadFactor Maximum load factor triggering rehash (default 1.0).
		 */
		UnorderedMap(usize initialSize = 16, f32 maxLoadFactor = 1.f) noexcept
			: m_Buckets(nullptr), m_BucketCount(0),
			  m_ElementCount(0), m_MaxLoadFactor(maxLoadFactor),
			  m_Hasher(HasherType()), m_NodeAllocator(NodeAllocator()), m_BucketAllocator(BucketAllocator())
		{
			AllocateBuckets(initialSize);
		}

		/** @brief Copy constructor. */
		UnorderedMap(const UnorderedMap& other) noexcept
		{
			AllocateBuckets(other.m_BucketCount);

			for (usize i = 0; i < other.m_BucketCount; ++i)
			{
				NodeType* current = other.m_Buckets[i];
				NodeType** tail = &m_Buckets[i];
				while (current)
				{
					NodeType* newNode = m_NodeAllocator.Allocate(1);
					Memory::Construct(newNode, current->Key(), current->Value());
					*tail = newNode;
					tail = &newNode->NextNode;
					current = current->NextNode;
				}
			}
			m_MaxLoadFactor = other.m_MaxLoadFactor;
			m_ElementCount = other.m_ElementCount;
			m_Hasher = other.m_Hasher;
			m_NodeAllocator = other.m_NodeAllocator;
			m_BucketAllocator = other.m_BucketAllocator;
		}

		/** @brief Move constructor. */
		UnorderedMap(UnorderedMap&& other) noexcept
			: m_Buckets(other.m_Buckets), m_BucketCount(other.m_BucketCount),
			m_ElementCount(other.m_ElementCount), m_MaxLoadFactor(other.m_MaxLoadFactor),
			m_Hasher(Move(other.m_Hasher)), m_NodeAllocator(Move(other.m_NodeAllocator)), m_BucketAllocator(Move(other.m_BucketAllocator))
		{
			other.m_Buckets = nullptr;
			other.m_ElementCount = 0;
			other.m_BucketCount = 0;
		}

		/** @brief Destructor. Destroys all nodes and deallocates bucket storage. */
		~UnorderedMap() noexcept
		{
			DeallocateBuckets();
		}

	public:
		/** @brief Returns number of elements in map. */
		usize Size() const noexcept { return m_ElementCount; }
		/** @brief Returns pointer to array of bucket heads. */
		const NodeType** Extract() const noexcept { return m_Buckets; }
		/** @brief Checks whether map is empty. */
		bool Empty() const noexcept { return m_ElementCount == 0; }

	public:
		/** @brief Returns an iterator to the first element. */
		Iterator Begin() noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return Iterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return Iterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}
		/** @brief Returns a const iterator to the first element. */
		ConstIterator Begin() const noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return ConstIterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}

		/** @brief Returns a const iterator to the first element. */
		ConstIterator CBegin() const noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return ConstIterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}
		/** @brief Returns an iterator to the end. */
		Iterator End() const noexcept { return Iterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount); }
		/** @brief Returns a const iterator to the end. */
		ConstIterator CEnd() const noexcept { return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount); }

	public:
		/** @brief Range-for compatible begin iterator. */
		Iterator begin() noexcept { return Begin(); }
		/** @brief Range-for compatible const begin iterator. */
		ConstIterator begin() const noexcept { return CBegin(); }
		/** @brief Range-for compatible const begin iterator. */
		ConstIterator cbegin() const noexcept { return CBegin(); }
		/** @brief Range-for compatible end iterator. */
		Iterator end() noexcept { return End(); }
		/** @brief Range-for compatible const end iterator. */
		ConstIterator end() const noexcept { return CEnd(); }
		/** @brief Range-for compatible const end iterator. */
		ConstIterator cend() const noexcept { return CEnd(); }

	public:
		/** @brief Copy assignment operator. */
		UnorderedMap& operator=(const UnorderedMap& other) noexcept
		{
			if (this != &other)
			{
				DeallocateBuckets();
				m_MaxLoadFactor = other.m_MaxLoadFactor;
				AllocateBuckets(other.m_BucketCount);
				for (usize i = 0; i < other.m_BucketCount; ++i)
				{
					NodeType* current = other.m_Buckets[i];
					NodeType** tail = &m_Buckets[i];
					while (current)
					{
						NodeType* newNode = m_NodeAllocator.Allocate(1);
						Memory::Construct(newNode, current->Key(), current->Value());
						*tail = newNode;
						tail = &newNode->NextNode;
						current = current->NextNode;
					}
				}
				m_ElementCount = other.m_ElementCount;
				m_Hasher = other.m_Hasher;
				m_NodeAllocator = other.m_NodeAllocator;
				m_BucketAllocator = other.m_BucketAllocator;
			}
			return *this;
		}

		/** @brief Move assignment operator. */
		UnorderedMap& operator=(UnorderedMap&& other) noexcept
		{
			if (this != &other)
			{
				DeallocateBuckets();

				m_Buckets = other.m_Buckets;
				m_ElementCount = other.m_ElementCount;
				m_BucketCount = other.m_BucketCount;
				m_MaxLoadFactor = other.m_MaxLoadFactor;
				m_Hasher = Move(other.m_Hasher);
				m_NodeAllocator = Move(other.m_NodeAllocator);
				m_BucketAllocator = Move(other.m_BucketAllocator);

				other.m_Buckets = nullptr;
				other.m_ElementCount = 0;
				other.m_BucketCount = 0;
			}
			return *this;
		}

	public:
		/**
		 * @brief Checks if map contains an element with @p key.
		 * @param key Key to search for.
		 * @return True if found.
		 */
		bool Contains(const KeyType& key) const noexcept { usize index; return FindImpl(key, index) != nullptr; }

		/**
		 * @brief Accesses element matching @p key.
		 * @param key Key to find.
		 * @return Mutable reference to key-value pair.
		 * @note Asserts key exists in debug builds.
		 */
		RefType At(const KeyType& key) noexcept
		{
			usize index;
			NodeType* node = FindImpl(key, index);
			MOON_CHECK(node, "Key not found in UnorderedMap");
			return node->Pair;
		}

		/**
		 * @brief Accesses element matching @p key (const overload).
		 * @param key Key to find.
		 * @return Const reference to key-value pair.
		 * @note Asserts key exists in debug builds.
		 */
		ConstRefType At(const KeyType& key) const noexcept
		{
			usize index;
			NodeType* node = FindImpl(key, index);
			MOON_CHECK(node, "Key not found in UnorderedMap");
			return node->Pair;
		}

		/** @brief Inserts key-value pair by copy. */
		Utility::Pair<Iterator, bool> Insert(const KeyType& key, const MappedType& value) noexcept { return InsertImpl({ key, value }); }
		/** @brief Inserts key-value pair by move. */
		Utility::Pair<Iterator, bool> Insert(KeyType&& key, MappedType&& value) noexcept { return InsertImpl({ Forward<KeyType>(key), Forward<MappedType>(value) }); }
		/** @brief Inserts pair by copy. */
		Utility::Pair<Iterator, bool> Insert(const ValueType& pair) noexcept { return InsertImpl(pair); }
		/** @brief Inserts pair by move. */
		Utility::Pair<Iterator, bool> Insert(ValueType&& pair) noexcept { return InsertImpl(Forward<ValueType>(pair)); }

		/**
		 * @brief Finds element with key equivalent to @p key.
		 * @param key Key to search for.
		 * @return Iterator to element, or `End()` if not found.
		 */
		Iterator Find(const KeyType& key) noexcept
		{
		    usize index; 
		    PtrType node = FindImpl(key, index); 
		    return node ? Iterator(node, m_Buckets, m_BucketCount, index) : End();
		}

		/**
		 * @brief Finds element with key equivalent to @p key (const overload).
		 * @param key Key to search for.
		 * @return Const iterator to element, or `CEnd()` if not found.
		 */
		ConstIterator Find(const KeyType& key) const noexcept
		{
			usize index;
			PtrType node = FindImpl(key, index);
			return node ? ConstIterator(node, m_Buckets, m_BucketCount, index) : CEnd();
		}

		/** @brief Erases element matching @p key. */
		void Erase(const KeyType& key) noexcept { EraseImpl(key); }
		/** @brief Erases element pointed to by iterator. */
		void Erase(Iterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }
		/** @brief Erases element pointed to by const iterator. */
		void Erase(ConstIterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }

		/** @brief Clears all elements from map. */
		void Clear() noexcept { ClearImpl(); }
		/** @brief Ensures bucket capacity is at least @p capacity. */
		bool Reserve(usize capacity) noexcept { return ReserveImpl(capacity); }
		/** @brief Sets bucket count to @p bucketCount and rehashes. */
		void Rehash(usize bucketCount) noexcept { RehashImpl(bucketCount); }

	public:
		/**
		 * @brief Subscript access operator. Returns reference to value for @p key, inserting default value if missing.
		 * @param key Key to access or insert.
		 * @return Reference to mapped value.
		 */
		MappedType& operator[](const KeyType& key) noexcept
		{
			usize index;
            if (NodeType* node = FindImpl(key, index)) return node->Pair.Value;

			auto [it, inserted] = InsertImpl({ key, MappedType() });
			return it->Value;
		}

	private:
		Utility::Pair<Iterator, bool> InsertImpl(const ValueType& pair) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(pair.Key);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, pair.Key))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, pair.Key, pair.Value);
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		Utility::Pair<Iterator, bool> InsertImpl(ValueType&& pair) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(pair.Key);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, pair.Key))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, Move(pair.Key), Move(pair.Value));
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		NodeType* FindImpl(const KeyType& key, usize& index) const noexcept
		{
			if (m_ElementCount <= 0) return nullptr;
			index = GetBucketIndex(key);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (current->Key() == key) return current;
				current = current->NextNode;
			}
			return nullptr;
		}

		void EraseImpl(const KeyType& key) noexcept
		{
			usize index = GetBucketIndex(key);
			NodeType* current = m_Buckets[index];
			NodeType* prev = nullptr;

			while (current)
			{
				if (current->Key() == key)
				{
					if (prev) prev->NextNode = current->NextNode;
					else m_Buckets[index] = current->NextNode;

					Memory::Destroy(current);
					m_NodeAllocator.Deallocate(current);
					m_ElementCount--;
					return;
				}
				prev = current;
				current = current->NextNode;
			}
		}

		void EraseImpl(NodeType* target, usize index) noexcept
		{
			if (!target) return;

			NodeType* current = m_Buckets[index];
			NodeType* prev = nullptr;

			while (current)
			{
				if (current == target)
				{
					if (prev) prev->NextNode = current->NextNode;
					else m_Buckets[index] = current->NextNode;

					Memory::Destroy(current);
					m_NodeAllocator.Deallocate(current);
					m_ElementCount--;
					return;
				}
				prev = current;
				current = current->NextNode;
			}
		}

		bool ReserveImpl(usize capacity) noexcept
		{
			if (capacity <= m_BucketCount) return false;
			RehashImpl(capacity);
			return true;
		}
		
	    void RehashImpl(usize numBuckets) noexcept
		{
			usize newSize = numBuckets;

			NodeType** newBuckets = m_BucketAllocator.Allocate(newSize);
			for (usize i = 0; i < newSize; ++i) newBuckets[i] = nullptr;

			for (usize i = 0; i < m_BucketCount; ++i)
			{
				NodeType* current = m_Buckets[i];
				while (current)
				{
					NodeType* next = current->NextNode;
					usize new_index = m_Hasher.Hash(&current->Key(), 1, newSize, newSize);
					current->NextNode = newBuckets[new_index];
					newBuckets[new_index] = current;
					current = next;
				}
			}

			m_BucketAllocator.Deallocate(m_Buckets, m_BucketCount);
			m_Buckets = newBuckets;
			m_BucketCount = newSize;
		}

		void ClearImpl() noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i)
			{
				NodeType* current = m_Buckets[i];
				while (current)
				{
					NodeType* next = current->NextNode;
					Memory::Destroy(current);
					m_NodeAllocator.Deallocate(current);
					current = next;
				}
				m_Buckets[i] = nullptr;
			}
			m_ElementCount = 0;
		}

	private:
		bool Compare(NodeType* node, const KeyType& key) const noexcept
		{
			if constexpr (IsSame<KeyType, const char7*> || IsSame<KeyType, const char8*>)
				return strcmp(AS_CHAR7(node->Key()), key) == 0;
			else if constexpr (IsSame<KeyType, const wchar*>)
				return wcscmp(node->Key(), key) == 0;
		    return node->Key() == key;
		}

		usize GetBucketIndex(const KeyType& key) const noexcept
		{
			return m_Hasher.Hash(&key, 1, m_BucketCount, m_BucketCount);
		}

		void AllocateBuckets(usize newBucketCount) noexcept
		{
			m_Buckets = m_BucketAllocator.Allocate(newBucketCount);
			for (usize i = 0; i < newBucketCount; ++i) m_Buckets[i] = nullptr;
			m_BucketCount = newBucketCount;
		}

		void DeallocateBuckets() noexcept
		{
			if (m_Buckets)
			{
				ClearImpl();
				m_BucketAllocator.Deallocate(m_Buckets, m_BucketCount);
				m_Buckets = nullptr;
			}
		}

		void CheckAndRehash() noexcept
		{
			if (static_cast<f32>(m_ElementCount + 1) / m_BucketCount > m_MaxLoadFactor) RehashImpl(m_BucketCount * REHASH_MULTIPLIER);
		}

	private:
		NodeType** m_Buckets;
        usize m_BucketCount;
        usize m_ElementCount;

        f32 m_MaxLoadFactor;

	private:
		HasherType m_Hasher;
		NodeAllocator m_NodeAllocator;
		BucketAllocator m_BucketAllocator;
	};
	
	/**
	 * @brief Specialization of `MC::UnorderedMap` optimized for `MC::String` keys.
	 *
	 * Utilizes dedicated string hashing and string comparison routines for optimal performance.
	 *
	 * @tparam Value Mapped value type.
	 * @tparam Hasher Hash functor template.
	 * @tparam Allocator Allocator policy template.
	 */
	template<typename Value, template<typename> class Hasher, template<typename> class Allocator>
	class UnorderedMap<MC::String, Value, Hasher, Allocator>
	{
		using Traits = Helper::UnorderedMapTraits<MC::String, Value, Hasher, Allocator>;

	public:
		using KeyType	   = typename Traits::KeyType;
		using MappedType   = typename Traits::MappedType;
		using ValueType	   = typename Traits::ValueType;
		using NodeType	   = typename Traits::NodeType;
		using PtrType	   = typename Traits::PtrType;
		using ConstPtrType = typename Traits::ConstPtrType;
		using RefType	   = typename Traits::RefType;
		using ConstRefType = typename Traits::ConstRefType;

	public:
		using HasherType	  = ExtractRebind<typename Traits::HasherType, const char8*>;
		using NodeAllocator   = typename Traits::NodeAllocator;
		using BucketAllocator = typename Traits::BucketAllocator;

	public:
		using Iterator      = UMapIterator<UnorderedMap, false>;
		using ConstIterator = UMapIterator<UnorderedMap, true>;

	public:
		static constexpr usize REHASH_MULTIPLIER = 2;

	public:
		UnorderedMap(usize initialSize = 16, f32 maxLoadFactor = 1.f) noexcept
			: m_Buckets(nullptr), m_BucketCount(0),
			m_ElementCount(0), m_MaxLoadFactor(maxLoadFactor),
			m_Hasher(HasherType()), m_NodeAllocator(NodeAllocator()), m_BucketAllocator(BucketAllocator())
		{
			AllocateBuckets(initialSize);
		}

		UnorderedMap(const UnorderedMap& other) noexcept
		{
			AllocateBuckets(other.m_BucketCount);

			for (usize i = 0; i < other.m_BucketCount; ++i)
			{
				NodeType* current = other.m_Buckets[i];
				NodeType** tail = &m_Buckets[i];
				while (current)
				{
					NodeType* newNode = m_NodeAllocator.Allocate(1);
					Memory::Construct(newNode, current->Key(), current->Value());
					*tail = newNode;
					tail = &newNode->NextNode;
					current = current->NextNode;
				}
			}
			m_ElementCount = other.m_ElementCount;
			m_Hasher = other.m_Hasher;
			m_NodeAllocator = other.m_NodeAllocator;
			m_BucketAllocator = other.m_BucketAllocator;
		}

		UnorderedMap(UnorderedMap&& other) noexcept
			: m_Buckets(other.m_Buckets), m_BucketCount(other.m_BucketCount),
			m_ElementCount(other.m_ElementCount), m_MaxLoadFactor(other.m_MaxLoadFactor),
			m_Hasher(Move(other.m_Hasher)), m_NodeAllocator(Move(other.m_NodeAllocator)), m_BucketAllocator(Move(other.m_BucketAllocator))
		{
			other.m_Buckets = nullptr;
			other.m_ElementCount = 0;
			other.m_BucketCount = 0;
		}

		~UnorderedMap() noexcept
		{
			DeallocateBuckets();
		}

	public:
		usize Size() const noexcept { return m_ElementCount; }
		const NodeType** Extract() const noexcept { return m_Buckets; }
		bool Empty() const noexcept { return m_ElementCount == 0; }

	public:
		Iterator Begin() noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return Iterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return Iterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}
		ConstIterator Begin() const noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return ConstIterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}

		ConstIterator CBegin() const noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return ConstIterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}
		Iterator End() const noexcept { return Iterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount); }
		ConstIterator CEnd() const noexcept { return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount); }

	public:
		Iterator begin() noexcept { return Begin(); }
		ConstIterator begin() const noexcept { return Begin(); }
		ConstIterator cbegin() const noexcept { return CBegin(); }
		Iterator end() const noexcept { return End(); }
		ConstIterator cend() const noexcept { return CEnd(); }

	public:
		UnorderedMap& operator=(const UnorderedMap& other) noexcept
		{
			if (this != &other)
			{
				DeallocateBuckets();
				m_MaxLoadFactor = other.m_MaxLoadFactor;
				AllocateBuckets(other.m_BucketCount);
				for (usize i = 0; i < other.m_BucketCount; ++i)
				{
					NodeType* current = other.m_Buckets[i];
					NodeType** tail = &m_Buckets[i];
					while (current)
					{
						NodeType* newNode = m_NodeAllocator.Allocate(1);
						Memory::Construct(newNode, current->Key(), current->Value());
						*tail = newNode;
						tail = &newNode->NextNode;
						current = current->NextNode;
					}
				}
				m_ElementCount = other.m_ElementCount;
				m_Hasher = other.m_Hasher;
				m_NodeAllocator = other.m_NodeAllocator;
				m_BucketAllocator = other.m_BucketAllocator;
			}
			return *this;
		}

		UnorderedMap& operator=(UnorderedMap&& other) noexcept
		{
			if (this != &other)
			{
				DeallocateBuckets();

				m_Buckets = other.m_Buckets;
				m_ElementCount = other.m_ElementCount;
				m_BucketCount = other.m_BucketCount;
				m_MaxLoadFactor = other.m_MaxLoadFactor;
				m_Hasher = Move(other.m_Hasher);
				m_NodeAllocator = Move(other.m_NodeAllocator);
				m_BucketAllocator = Move(other.m_BucketAllocator);

				other.m_Buckets = nullptr;
				other.m_ElementCount = 0;
				other.m_BucketCount = 0;
			}
			return *this;
		}

	public:
		bool Contains(const KeyType& key) const noexcept { return Contains(StringView(key)); }
		bool Contains(const MC::StringView& key) const noexcept { usize index; return FindImpl(key, index) != nullptr; }

		RefType At(const KeyType& key) noexcept { return At(StringView(key)); }
		RefType At(const MC::StringView& key) noexcept
		{
			usize index;
			NodeType* node = FindImpl(key, index);
			MOON_CHECK(node, "Key not found in UnorderedMap");
			return node->Pair;
		}

		Utility::Pair<Iterator, bool> Insert(const KeyType& key, const MappedType& value) noexcept { return InsertImpl({ key, value }); }
		Utility::Pair<Iterator, bool> Insert(KeyType&& key, MappedType&& value) noexcept { return InsertImpl({ Forward<KeyType>(key), Forward<MappedType>(value) }); }
		Utility::Pair<Iterator, bool> Insert(const MC::StringView& key, const MappedType& value) noexcept { return InsertImpl({ key.ToString(), value }); }
		Utility::Pair<Iterator, bool> Insert(MC::StringView&& key, MappedType&& value) noexcept { return InsertImpl({ Forward<KeyType>(key.ToString()), Forward<MappedType>(value) }); }
		Utility::Pair<Iterator, bool> Insert(const ValueType& pair) noexcept { return InsertImpl(pair); }
		Utility::Pair<Iterator, bool> Insert(ValueType&& pair) noexcept { return InsertImpl(Forward<ValueType>(pair)); }

		Iterator Find(const KeyType& key) noexcept { return Find(StringView(key)); }
		Iterator Find(const MC::StringView& key) noexcept
		{
			usize index;
			PtrType node = FindImpl(key, index);
			return node ? Iterator(node, m_Buckets, m_BucketCount, index) : End();
		}

		ConstIterator Find(const KeyType& key) const noexcept { return Find(StringView(key)); }
		ConstIterator Find(const MC::StringView& key) const noexcept
		{
			usize index;
			PtrType node = FindImpl(key, index);
			return node ? ConstIterator(node, m_Buckets, m_BucketCount, index) : CEnd();

		}

		void Erase(const KeyType& key) noexcept { EraseImpl(key); }
		void Erase(const MC::StringView& key) noexcept { EraseImpl(key); }
		void Erase(Iterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }
		void Erase(ConstIterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }

		void Clear() noexcept { ClearImpl(); }
		bool Reserve(usize capacity) noexcept { return ReserveImpl(capacity); }
		void Rehash(usize bucketCount) noexcept { RehashImpl(bucketCount); }

	public:
		MappedType& operator[](const StringView& key) noexcept
		{
			usize index;
			if (NodeType* node = FindImpl(key, index)) return node->Pair.Value;

			auto [it, inserted] = InsertImpl({ key.String(), MappedType() });
			return it->Value;
		}

	private:
		Utility::Pair<Iterator, bool> InsertImpl(const ValueType& pair) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(pair.Key);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, pair.Key))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, pair.Key, pair.Value);
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		Utility::Pair<Iterator, bool> InsertImpl(ValueType&& pair) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(pair.Key);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, pair.Key))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, Move(pair.Key), Move(pair.Value));
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		NodeType* FindImpl(const StringView& key, usize& index) const noexcept
		{
			if (m_ElementCount <= 0) return nullptr;
			index = GetBucketIndex(key);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (current->Key() == key) return current;
				current = current->NextNode;
			}
			return nullptr;
		}

		void EraseImpl(const StringView& key) noexcept
		{
			usize index = GetBucketIndex(key);
			NodeType* current = m_Buckets[index];
			NodeType* prev = nullptr;

			while (current)
			{
				if (Compare(current, key))
				{
					if (prev) prev->NextNode = current->NextNode;
					else m_Buckets[index] = current->NextNode;

					Memory::Destroy(current);
					m_NodeAllocator.Deallocate(current);
					m_ElementCount--;
					return;
				}
				prev = current;
				current = current->NextNode;
			}
		}

		void EraseImpl(NodeType* target, usize index) noexcept
		{
			if (!target) return;

			NodeType* current = m_Buckets[index];
			NodeType* prev = nullptr;

			while (current)
			{
				if (current == target)
				{
					if (prev) prev->NextNode = current->NextNode;
					else m_Buckets[index] = current->NextNode;

					Memory::Destroy(current);
					m_NodeAllocator.Deallocate(current);
					m_ElementCount--;
					return;
				}
				prev = current;
				current = current->NextNode;
			}
		}

		bool ReserveImpl(usize capacity) noexcept
		{
			if (capacity <= m_BucketCount) return false;
			RehashImpl(capacity);
			return true;
		}

		void RehashImpl(usize numBuckets) noexcept
		{
			usize newSize = numBuckets;

			NodeType** newBuckets = m_BucketAllocator.Allocate(newSize);
			for (usize i = 0; i < newSize; ++i) newBuckets[i] = nullptr;

			for (usize i = 0; i < m_BucketCount; ++i)
			{
				NodeType* current = m_Buckets[i];
				while (current)
				{
					NodeType* next = current->NextNode;
					usize new_index = m_Hasher.Hash(current->Key().String(), current->Key().Size(), newSize, newSize);
					current->NextNode = newBuckets[new_index];
					newBuckets[new_index] = current;
					current = next;
				}
			}

			m_BucketAllocator.Deallocate(m_Buckets, m_BucketCount);
			m_Buckets = newBuckets;
			m_BucketCount = newSize;
		}

		void ClearImpl() noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i)
			{
				NodeType* current = m_Buckets[i];
				while (current)
				{
					NodeType* next = current->NextNode;
					Memory::Destroy(current);
					m_NodeAllocator.Deallocate(current);
					current = next;
				}
				m_Buckets[i] = nullptr;
			}
			m_ElementCount = 0;
		}

	private:
		static bool Compare(NodeType* node, const KeyType& key) noexcept
		{
			return node->Key() == key;
		}

		usize GetBucketIndex(const StringView& key) const noexcept
		{
			return m_Hasher.Hash(key.String(), key.Size(), m_BucketCount, m_BucketCount);
		}

		void AllocateBuckets(usize newBucketCount) noexcept
		{
			m_Buckets = m_BucketAllocator.Allocate(newBucketCount);
			for (usize i = 0; i < newBucketCount; ++i) m_Buckets[i] = nullptr;
			m_BucketCount = newBucketCount;
		}

		void DeallocateBuckets() noexcept
		{
			if (m_Buckets)
			{
				ClearImpl();
				m_BucketAllocator.Deallocate(m_Buckets, m_BucketCount);
				m_Buckets = nullptr;
			}
		}

		void CheckAndRehash() noexcept
		{
			if (static_cast<f32>(m_ElementCount + 1) / m_BucketCount > m_MaxLoadFactor) RehashImpl(m_BucketCount * REHASH_MULTIPLIER);
		}

	private:
		NodeType** m_Buckets;
		usize m_BucketCount;
		usize m_ElementCount;

		f32 m_MaxLoadFactor;

	private:
		HasherType m_Hasher;
		NodeAllocator m_NodeAllocator;
		BucketAllocator m_BucketAllocator;
	};
}