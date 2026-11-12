#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Pair.hpp"
#include "MoonCore/Memory/Hasher.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"
#include "MoonCore/Containers/String.hpp"

namespace MC
{
	template <typename Container, bool IsConst = false>
	class UMapIterator
	{
	public:
		using KeyType		= typename Container::KeyType;
		using ValueType		= typename Container::ValueType;
		using DiffType		= iptr;
		using PtrType		= typename Container::NodeType*;
		using ValuePtrType	= Conditional<IsConst, const ValueType*, ValueType*>;
		using RefType		= Conditional<IsConst, const ValueType&, ValueType&>;
		using Category		= Iterator::Tags::Forward;

	public:
		using key_type = KeyType;
		using value_type = ValueType;
		using difference_type = DiffType;
		using pointer = PtrType;
		using reference = RefType;
		using iterator_category = Category;

	public:
		constexpr UMapIterator() noexcept
		    : m_Ptr(nullptr), m_Buckets(nullptr), m_BucketCount(0), m_BucketIndex(0)
		{}

		constexpr UMapIterator(PtrType ptr, PtrType* buckets, usize bucketCount, usize bucketIndex)
			: m_Ptr(ptr), m_Buckets(buckets), m_BucketCount(bucketCount), m_BucketIndex(bucketIndex)
		{}

	public:
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

		constexpr UMapIterator operator++(int) noexcept
		{
			UMapIterator temp = *this;
			++(*this);
			return temp;
		}

		constexpr bool operator==(const UMapIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		constexpr bool operator!=(const UMapIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }

		constexpr ValuePtrType operator->() const noexcept { return &m_Ptr->Pair; }
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
		template <typename KeyT, typename ValueT>
		struct MapNode
		{
			using KeyType = KeyT;
			using ValueType = ValueT;
			using PairType = Utility::Pair<const KeyType, ValueType>;

		public:
			PairType Pair;
			MapNode* NextNode;

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
		template<typename Key, typename Value, template<typename> class Hasher, template<typename> class Allocator>
		concept TemplateArgumentsEligibleForUnorderedMap = 
			IsDefaultConstructible<Key> && IsDefaultConstructible<Value> &&
			IsNothrowCopyConstructible<Key> && IsNothrowCopyConstructible<Value> &&
			IsNothrowMoveConstructible<Key> && IsNothrowMoveConstructible<Value> &&
			Concept::HasherType<typename MC::Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>::HasherType>&&
			Concept::HeapAllocatorType<typename MC::Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>::NodeAllocator>&&
			Concept::HeapAllocatorType<typename MC::Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>::BucketAllocator>;
	}

	template <typename Key, typename Value,
		template<typename> class Hasher = Memory::Hasher,
		template<typename> class Allocator = Memory::HeapAllocator>
		requires Concept::TemplateArgumentsEligibleForUnorderedMap<Key, Value, Hasher, Allocator>
    class UnorderedMap
	{
	    using Traits = Helper::UnorderedMapTraits<Key, Value, Hasher, Allocator>;

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
		using HasherType	  = typename Traits::HasherType;
		using NodeAllocator	  = typename Traits::NodeAllocator;
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
			m_MaxLoadFactor = other.m_MaxLoadFactor;
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
		ConstIterator begin() const noexcept { return CBegin(); }
		ConstIterator cbegin() const noexcept { return CBegin(); }
		Iterator end() noexcept { return End(); }
		ConstIterator end() const noexcept { return CEnd(); }
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
		bool Contains(const KeyType& key) const noexcept { usize index; return FindImpl(key, index) != nullptr; }

		RefType At(const KeyType& key) noexcept
		{
			usize index;
			NodeType* node = FindImpl(key, index);
			MC_CHECK(node, "Key not found in UnorderedMap");
			return node->Pair;
		}

		ConstRefType At(const KeyType& key) const noexcept
		{
			usize index;
			NodeType* node = FindImpl(key, index);
			MC_CHECK(node, "Key not found in UnorderedMap");
			return node->Pair;
		}

		Utility::Pair<Iterator, bool> Insert(const KeyType& key, const MappedType& value) noexcept { return InsertImpl({ key, value }); }
		Utility::Pair<Iterator, bool> Insert(KeyType&& key, MappedType&& value) noexcept { return InsertImpl({ Forward<KeyType>(key), Forward<MappedType>(value) }); }
		Utility::Pair<Iterator, bool> Insert(const ValueType& pair) noexcept { return InsertImpl(pair); }
		Utility::Pair<Iterator, bool> Insert(ValueType&& pair) noexcept { return InsertImpl(Forward<ValueType>(pair)); }

		Iterator Find(const KeyType& key) noexcept
		{
		    usize index; 
		    PtrType node = FindImpl(key, index); 
		    return node ? Iterator(node, m_Buckets, m_BucketCount, index) : End();
		}
		ConstIterator Find(const KeyType& key) const noexcept
		{
			usize index;
			PtrType node = FindImpl(key, index);
			return node ? ConstIterator(node, m_Buckets, m_BucketCount, index) : CEnd();
		}

		void Erase(const KeyType& key) noexcept { EraseImpl(key); }
		void Erase(Iterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }
		void Erase(ConstIterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }

		void Clear() noexcept { ClearImpl(); }
		bool Reserve(usize capacity) noexcept { return ReserveImpl(capacity); }
		void Rehash(usize bucketCount) noexcept { RehashImpl(bucketCount); }

	public:
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
			MC_CHECK(node, "Key not found in UnorderedMap");
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