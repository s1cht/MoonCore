#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Pair.hpp"
#include "MoonCore/Memory/Hasher.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Memory/Allocators/Heap/HeapAllocator.hpp"

namespace MC
{
	template <typename Container, bool IsConst = false>
	class USetIterator
	{
	public:
		using KeyType = Container::KeyType;
		using ValueType = Container::ValueType;
		using DiffType = iptr;
		using PtrType = Container::NodeType*;
		using ValuePtrType = Conditional<IsConst, const ValueType*, ValueType*>;
		using RefType = Conditional<IsConst, const ValueType&, ValueType&>;
		using Category = Iterator::Tags::Forward;

	public:
		using key_type = KeyType;
		using value_type = ValueType;
		using difference_type = DiffType;
		using pointer = PtrType;
		using reference = RefType;
		using iterator_category = Category;

	public:
		constexpr USetIterator() noexcept
			: m_Ptr(nullptr), m_Buckets(nullptr), m_BucketCount(0), m_BucketIndex(0)
		{}

		constexpr USetIterator(PtrType ptr, PtrType* buckets, usize bucketCount, usize bucketIndex)
			: m_Ptr(ptr), m_Buckets(buckets), m_BucketCount(bucketCount), m_BucketIndex(bucketIndex)
		{}

	public:
		constexpr USetIterator& operator++() noexcept
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

		constexpr USetIterator operator++(int) noexcept
		{
			USetIterator temp = *this;
			++(*this);
			return temp;
		}

		constexpr bool operator==(const USetIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		constexpr bool operator!=(const USetIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }

		constexpr ValuePtrType operator->() const noexcept { return &m_Ptr->Value; }
		constexpr RefType operator*() const noexcept { return m_Ptr->Value; }

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
		template <typename ValueT>
		struct SetNode
		{
			using ValueType = ValueT;

		public:
			const ValueType Value;
			SetNode* NextNode;

			constexpr SetNode(const ValueType& value) noexcept
				: Value(value), NextNode(nullptr)
			{}

			constexpr SetNode(ValueType&& value) noexcept
				: Value(Move(value)), NextNode(nullptr)
			{}
		};

		template<typename Value,
			template<typename> class Hasher = Memory::Hasher,
			template<typename> class Allocator = Memory::HeapAllocator>
		struct UnorderedSetTraits
		{
		public:
			using KeyType = Value;
			using MappedType = Value;
			using ValueType = const KeyType;
			using NodeType = SetNode<KeyType>;
			using PtrType = NodeType*;
			using ConstPtrType = const NodeType*;
			using RefType = ValueType&;
			using ConstRefType = const ValueType&;

		public:
			using HasherType = Hasher<KeyType>;
			using NodeAllocator = Allocator<NodeType>;
			using BucketAllocator = Allocator<NodeType*>;
		};
	}

	namespace Concept
	{
		template<typename Value, template<typename> class Hasher, template<typename> class Allocator>
		concept TemplateArgumentsEligibleForUnorderedSet =
			IsDefaultConstructible<Value> &&
			IsNothrowCopyConstructible<Value> &&
			IsNothrowMoveConstructible<Value> &&
			Concept::HasherType<typename Core::Helper::UnorderedSetTraits<Value, Hasher, Allocator>::HasherType>&&
			Concept::HeapAllocatorType<typename Core::Helper::UnorderedSetTraits<Value, Hasher, Allocator>::NodeAllocator>&&
			Concept::HeapAllocatorType<typename Core::Helper::UnorderedSetTraits<Value, Hasher, Allocator>::BucketAllocator>;
	}

	template<typename Value,
		template<typename> class Hasher = Memory::Hasher,
		template<typename> class Allocator = Memory::HeapAllocator>
		requires Concept::TemplateArgumentsEligibleForUnorderedSet<Value, Hasher, Allocator>
    class UnorderedSet
	{
		using Traits = Helper::UnorderedSetTraits<Value, Hasher, Allocator>;

	public:
		using KeyType	   = Traits::KeyType;
		using MappedType   = Traits::MappedType;
		using ValueType	   = Traits::ValueType;
		using NodeType	   = Traits::NodeType;
		using PtrType	   = Traits::PtrType;
		using ConstPtrType = Traits::ConstPtrType;
		using RefType	   = Traits::RefType;
		using ConstRefType = Traits::ConstRefType;

	public:
		using HasherType	  = Traits::HasherType;
		using NodeAllocator   = Traits::NodeAllocator;
		using BucketAllocator = Traits::BucketAllocator;

	public:
		using Iterator	    = USetIterator<UnorderedSet, false>;
		using ConstIterator = USetIterator<UnorderedSet, true>;

	public:
		static constexpr usize REHASH_MULTIPLIER = 2;

	public:
		UnorderedSet(usize initialSize = 16, f32 maxLoadFactor = 1.f) noexcept
			: m_Buckets(nullptr), m_BucketCount(0),
			  m_ElementCount(0), m_MaxLoadFactor(maxLoadFactor),
			  m_Hasher(HasherType()), m_NodeAllocator(NodeAllocator()), m_BucketAllocator(BucketAllocator())
		{
			AllocateBuckets(initialSize);
		}

		UnorderedSet(const UnorderedSet& other) noexcept
		{
			AllocateBuckets(other.m_BucketCount);

			for (usize i = 0; i < other.m_BucketCount; ++i)
			{
				NodeType* current = other.m_Buckets[i];
				NodeType** tail = &m_Buckets[i];
				while (current)
				{
					NodeType* newNode = m_NodeAllocator.Allocate(1);
					Memory::Construct(newNode, current->Value);
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

		UnorderedSet(UnorderedSet&& other) noexcept
			: m_Buckets(other.m_Buckets), m_BucketCount(other.m_BucketCount),
			m_ElementCount(other.m_ElementCount), m_MaxLoadFactor(other.m_MaxLoadFactor),
			m_Hasher(Move(other.m_Hasher)), m_NodeAllocator(Move(other.m_NodeAllocator)), m_BucketAllocator(Move(other.m_BucketAllocator))
		{
			other.m_Buckets = nullptr;
			other.m_ElementCount = 0;
			other.m_BucketCount = 0;
		}

		~UnorderedSet() noexcept
		{
			DeallocateBuckets();
		}

	public:
		usize Size() const noexcept { return m_ElementCount; }
		const NodeType** Extract() const noexcept { return m_Buckets; }
		bool Empty() const noexcept { return m_ElementCount == 0; }

	public:
		Iterator Begin() const noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return Iterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return Iterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
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
		UnorderedSet& operator=(const UnorderedSet& other) noexcept
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
						NodeType* new_node = m_NodeAllocator.Allocate(1);
						Memory::Construct(new_node, current->Value);
						*tail = new_node;
						tail = &new_node->NextNode;
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

		UnorderedSet& operator=(UnorderedSet&& other) noexcept
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
		bool Contains(const ValueType& value) const noexcept { usize index; return FindImpl(value, index) != nullptr; }

		RefType At(const ValueType& value) noexcept
		{
			usize index;
			NodeType* node = FindImpl(value, index);
			MC_CHECK(node, "Key not found in UnorderedMap");
			return node->Value;
		}

		Utility::Pair<Iterator, bool> Insert(const ValueType& value) noexcept { return InsertImpl(value); }
		Utility::Pair<Iterator, bool> Insert(ValueType&& value) noexcept { return InsertImpl(Forward<ValueType>(value)); }

		Iterator Find(const ValueType& value) noexcept
		{
			usize index;
			PtrType node = FindImpl(value, index);
			return node ? Iterator(node, m_Buckets, m_BucketCount, index) : End();
		}
		ConstIterator Find(const ValueType& value) const noexcept
		{
			usize index;
			PtrType node = FindImpl(value, index);
			return node ? ConstIterator(node, m_Buckets, m_BucketCount, index) : CEnd();
		}

		void Erase(const ValueType& value) noexcept { EraseImpl(value); }
		void Erase(Iterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }
		void Erase(ConstIterator iterator) noexcept { EraseImpl(iterator.m_Ptr, iterator.m_BucketIndex); }

		void Clear() noexcept { ClearImpl(); }
		bool Reserve(usize capacity) noexcept { return ReserveImpl(capacity); }
		void Rehash(usize bucketCount) noexcept { RehashImpl(bucketCount); }

	private:
		Utility::Pair<Iterator, bool> InsertImpl(const ValueType& value) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(value);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, value))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, value);
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		Utility::Pair<Iterator, bool> InsertImpl(ValueType&& value) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(value);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, value))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, Forward<ValueType>(value));
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		NodeType* FindImpl(const ValueType& value, usize& index) const noexcept
		{
			if (m_ElementCount <= 0) return nullptr;
			index = GetBucketIndex(value);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (current->Value == value) return current;
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
				if (current->Value == key)
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
					usize new_index = m_Hasher.Hash(&current->Value, 1, newSize, newSize);
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
		bool Compare(NodeType* node, const ValueType& value) const noexcept
		{
			if constexpr (IsSame<KeyType, const char7*> || IsSame<KeyType, const char8*>)
				return strcmp(AS_CHAR7(node->Value), value) == 0;
			else if constexpr (IsSame<KeyType, const wchar*>)
				return wcscmp(node->Value, value) == 0;
			return node->Value == value;
		}

		usize GetBucketIndex(const ValueType& value) const noexcept
		{
			return m_Hasher.Hash(&value, 1, m_BucketCount, m_BucketCount);
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

	template< template<typename> class Hasher, template<typename> class Allocator>
	class UnorderedSet<MC::String, Hasher, Allocator>
	{
		using Traits = Helper::UnorderedSetTraits<MC::String, Hasher, Allocator>;

	public:
		using KeyType		= Traits::KeyType;
		using MappedType	= Traits::MappedType;
		using ValueType		= Traits::ValueType;
		using NodeType		= Traits::NodeType;
		using PtrType		= Traits::PtrType;
		using ConstPtrType	= Traits::ConstPtrType;
		using RefType		= Traits::RefType;
		using ConstRefType	= Traits::ConstRefType;

	public:
		using HasherType		= ExtractRebind<typename Traits::HasherType, const char8*>;
		using NodeAllocator		= Traits::NodeAllocator;
		using BucketAllocator	= Traits::BucketAllocator;

	public:
		using Iterator		= USetIterator<UnorderedSet, false>;
		using ConstIterator = USetIterator<UnorderedSet, true>;

	public:
		static constexpr usize REHASH_MULTIPLIER = 2;

	public:
		UnorderedSet(usize initialSize = 16, f32 maxLoadFactor = 1.f) noexcept
			: m_Buckets(nullptr), m_BucketCount(0),
			m_ElementCount(0), m_MaxLoadFactor(maxLoadFactor),
			m_Hasher(HasherType()), m_NodeAllocator(NodeAllocator()), m_BucketAllocator(BucketAllocator())
		{
			AllocateBuckets(initialSize);
		}

		UnorderedSet(const UnorderedSet& other) noexcept
		{
			AllocateBuckets(other.m_BucketCount);

			for (usize i = 0; i < other.m_BucketCount; ++i)
			{
				NodeType* current = other.m_Buckets[i];
				NodeType** tail = &m_Buckets[i];
				while (current)
				{
					NodeType* newNode = m_NodeAllocator.Allocate(1);
					Memory::Construct(newNode, current->Value);
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

		UnorderedSet(UnorderedSet&& other) noexcept
			: m_Buckets(other.m_Buckets), m_BucketCount(other.m_BucketCount),
			m_ElementCount(other.m_ElementCount), m_MaxLoadFactor(other.m_MaxLoadFactor),
			m_Hasher(Move(other.m_Hasher)), m_NodeAllocator(Move(other.m_NodeAllocator)), m_BucketAllocator(Move(other.m_BucketAllocator))
		{
			other.m_Buckets = nullptr;
			other.m_ElementCount = 0;
			other.m_BucketCount = 0;
		}

		~UnorderedSet() noexcept
		{
			DeallocateBuckets();
		}

	public:
		usize Size() const noexcept { return m_ElementCount; }
		const NodeType** Extract() const noexcept { return m_Buckets; }
		bool Empty() const noexcept { return m_ElementCount == 0; }

	public:
		Iterator Begin() const noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return Iterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return Iterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}
		ConstIterator CBegin() const noexcept
		{
			for (usize i = 0; i < m_BucketCount; ++i) if (m_Buckets[i]) return ConstIterator(m_Buckets[i], m_Buckets, m_BucketCount, i);
			return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount);
		}
		Iterator End() const noexcept { return Iterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount); }
		ConstIterator CEnd() const noexcept { return ConstIterator(nullptr, m_Buckets, m_BucketCount, m_BucketCount); }

	public:
		Iterator begin() const noexcept { return Begin(); }
		ConstIterator cbegin() const noexcept { return CBegin(); }
		Iterator end() const noexcept { return End(); }
		ConstIterator cend() const noexcept { return CEnd(); }

	public:
		UnorderedSet& operator=(const UnorderedSet& other) noexcept
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
						NodeType* new_node = m_NodeAllocator.Allocate(1);
						Memory::Construct(new_node, current->Value);
						*tail = new_node;
						tail = &new_node->NextNode;
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

		UnorderedSet& operator=(UnorderedSet&& other) noexcept
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
		bool Contains(const ValueType& value) const noexcept { return Contains(StringView(value)); }
		bool Contains(const MC::StringView& value) const noexcept { usize index; return FindImpl(value, index) != nullptr; }

		RefType At(const ValueType& value) const noexcept { return At(StringView(value)); }
		RefType At(const MC::StringView& value) noexcept
		{
			usize index;
			NodeType* node = FindImpl(value, index);
			MC_CHECK(node, "Key not found in UnorderedSet");
			return node->Value;
		}

		Utility::Pair<Iterator, bool> Insert(const ValueType& value) noexcept { return InsertImpl(value); }
		Utility::Pair<Iterator, bool> Insert(const MC::StringView& value) noexcept { return InsertImpl(value.ToString()); }
		Utility::Pair<Iterator, bool> Insert(ValueType&& value) noexcept { return InsertImpl(Forward<ValueType>(value)); }
		Utility::Pair<Iterator, bool> Insert(MC::StringView&& value) noexcept { return InsertImpl(Forward<ValueType>(value.ToString())); }

		Iterator Find(const ValueType& value) noexcept { return Find(StringView(value)); }
		Iterator Find(const StringView& value) noexcept
		{
			usize index;
			PtrType node = FindImpl(value, index);
			return node ? Iterator(node, m_Buckets, m_BucketCount, index) : End();
		}
		ConstIterator Find(const ValueType& value) const noexcept { return Find(StringView(value)); }
		ConstIterator Find(const StringView& value) const noexcept
		{
			usize index;
			PtrType node = FindImpl(value, index);
			return node ? ConstIterator(node, m_Buckets, m_BucketCount, index) : CEnd();
		}

		void Erase(const ValueType& key) noexcept { EraseImpl(key); }
		void Erase(const MC::StringView& key) noexcept { EraseImpl(key); }
		void Erase(Iterator iterator) noexcept { EraseImpl(iterator.m_Ptr); }
		void Erase(ConstIterator iterator) noexcept { EraseImpl(iterator.m_Ptr); }

		void Clear() noexcept { ClearImpl(); }
		bool Reserve(usize capacity) noexcept { return ReserveImpl(capacity); }
		void Rehash(usize bucketCount) noexcept { RehashImpl(bucketCount); }

	private:
		Utility::Pair<Iterator, bool> InsertImpl(const ValueType& value) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(value);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, value))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, value);
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		Utility::Pair<Iterator, bool> InsertImpl(ValueType&& value) noexcept
		{
			CheckAndRehash();

			usize index = GetBucketIndex(value);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (Compare(current, value))
					return { Iterator(current, m_Buckets, m_BucketCount, index), false };
				current = current->NextNode;
			}

			NodeType* newNode = m_NodeAllocator.Allocate(1);
			Memory::Construct(newNode, Forward<ValueType>(value));
			newNode->NextNode = m_Buckets[index];
			m_Buckets[index] = newNode;
			m_ElementCount++;

			return { Iterator(newNode, m_Buckets, m_BucketCount, index), true };
		}

		NodeType* FindImpl(const MC::StringView& value, usize& index) const noexcept
		{
			if (m_ElementCount <= 0) return nullptr;
			index = GetBucketIndex(value);
			NodeType* current = m_Buckets[index];

			while (current)
			{
				if (current->Value == value) return current;
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
				if (current->Value == key)
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
					usize new_index = m_Hasher.Hash(current->Value.String(), current->Value.Size(), newSize, newSize);
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
		static bool Compare(NodeType* node, const ValueType& value) noexcept
		{
			return node->Value == value;
		}

		usize GetBucketIndex(const StringView& value) const noexcept
		{
			return m_Hasher.Hash(value.String(), value.Size(), m_BucketCount, m_BucketCount);
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