#pragma once
#include "Details/ArrayShared.hpp"
#include "MoonCore/Utility/Algorithm.hpp"
#include "MoonCore/Memory/Memory.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/InitializerList.hpp"
#include "MoonCore/Utility/Nullable.hpp"

namespace MC
{
	template<typename T, usize ArraySize>
		requires IsNothrowDestructible<T>
	class FixedArrayImpl
	{
	public:
		using ValueType = T;
		using PtrType = ValueType*;
		using ConstPtrType = const ValueType*;
		using RefType = ValueType&;
		using ConstRefType = const ValueType&;

		using Iterator = ArrayIterator<FixedArrayImpl, false>;
		using ConstIterator = ArrayIterator<FixedArrayImpl, true>;
		using ReverseIterator = MC::Iterator::ReverseIterator<Iterator>;
		using ConstReverseIterator = MC::Iterator::ReverseIterator<ConstIterator>;

	public:
		static constexpr usize FixedSize = ArraySize;

	public:
		constexpr FixedArrayImpl() noexcept {}

		constexpr FixedArrayImpl(const Utility::InitializerList<ValueType>& other) noexcept
		{
			MC_CHECK(other.size() <= ArraySize, "initializer_list overflows FixedArrayImpl");
			for (usize i = 0; i < FixedSize; i++)
				m_Data[i] = other.data()[i];
		}

		constexpr FixedArrayImpl(const FixedArrayImpl& other) noexcept
		{
			for (usize i = 0; i < FixedSize; i++)
				m_Data[i] = other.m_Data[i];
		}

		constexpr FixedArrayImpl(FixedArrayImpl&& other) noexcept
		{
			for (usize i = 0; i < FixedSize; i++)
				m_Data[i] = Move(other.m_Data[i]);
		}

		constexpr ~FixedArrayImpl() noexcept = default;

	public:
		static constexpr usize Size() noexcept { return FixedSize; }
		static constexpr usize Capacity() noexcept { return FixedSize; }
		constexpr ConstPtrType Data() const noexcept { return m_Data; }
		constexpr PtrType Data() noexcept { return m_Data; }
		
	    constexpr RefType Front() noexcept { return *m_Data; }
		constexpr ConstRefType Front() const noexcept { return *m_Data; }
		constexpr RefType Back() noexcept { return *(m_Data + FixedSize - 1); }
		constexpr ConstRefType Back() const noexcept { return *(m_Data + FixedSize - 1); }

	public:
		constexpr Iterator Begin() noexcept { return Iterator(m_Data); }
		constexpr ConstIterator Begin() const noexcept { return ConstIterator(m_Data); }
		constexpr ConstIterator CBegin() const noexcept { return ConstIterator(m_Data); }
		constexpr Iterator End() noexcept { return Iterator(m_Data + FixedSize); }
		constexpr ConstIterator End() const noexcept { return ConstIterator(m_Data + FixedSize); }
		constexpr ConstIterator CEnd() const noexcept { return ConstIterator(m_Data + FixedSize); }

		constexpr ReverseIterator RBegin() noexcept { return FixedSize > 0 ? ReverseIterator(m_Data + FixedSize - 1) : REnd(); }
		constexpr ConstReverseIterator RBegin() const noexcept { return FixedSize > 0 ? ReverseIterator(m_Data + FixedSize - 1) : REnd(); }
		constexpr ConstReverseIterator CRBegin() const noexcept { return FixedSize > 0 ? ConstReverseIterator(m_Data + FixedSize - 1) : CREnd(); }
		constexpr ReverseIterator REnd() noexcept { return ReverseIterator(m_Data - 1); }
		constexpr ConstReverseIterator REnd() const noexcept { return ReverseIterator(m_Data - 1); }
		constexpr ConstReverseIterator CREnd() const noexcept { return ConstReverseIterator(m_Data - 1); }

	public:
		constexpr Iterator begin() noexcept { return Begin(); }
		constexpr ConstIterator begin() const noexcept { return CBegin(); }
		constexpr ConstIterator cbegin() const noexcept { return CBegin(); }
		
	    constexpr Iterator end() noexcept { return End(); }
		constexpr ConstIterator end() const noexcept { return CEnd(); }
		constexpr ConstIterator cend() const noexcept { return CEnd(); }

		constexpr ReverseIterator rbegin() noexcept { return RBegin(); }
		constexpr ConstReverseIterator rbegin() const noexcept { return RBegin(); }
		constexpr ConstReverseIterator crbegin() const noexcept { return CRBegin(); }
		constexpr ReverseIterator rend() noexcept { return REnd(); }
		constexpr ConstReverseIterator rend() const noexcept { return REnd(); }
		constexpr ConstReverseIterator crend() const noexcept { return CREnd(); }

	public:
		constexpr FixedArrayImpl& operator=(const FixedArrayImpl& other) noexcept
		{
			if (this != &other)
			{
				for (usize i = 0; i < FixedSize; i++)
					Memory::Construct(&m_Data[i], other.m_Data[i]);
			}
			return *this;
		}

		constexpr FixedArrayImpl& operator=(FixedArrayImpl&& other) noexcept
		{
			if (this != &other)
			{
				for (usize i = 0; i < FixedSize; i++)
					Memory::Construct(&m_Data[i], Move(other.m_Data[i]));
			}
			return *this;
		}


		constexpr RefType operator[](const usize index) noexcept
		{
			MC_CHECK(index < FixedSize, "Index in array is out of range!");
			return m_Data[index];
		}

		constexpr ConstRefType operator[](const usize index) const noexcept
		{
			MC_CHECK(index < FixedSize, "Index in array is out of range!");
			return m_Data[index];
		}

		template<Concept::ScopedEnum Enum>
		constexpr RefType operator[](const Enum index) noexcept
		{
			MC_CHECK(static_cast<usize>(index) < FixedSize, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

		template<Concept::ScopedEnum Enum>
		constexpr ConstRefType operator[](const Enum index) const noexcept
		{
			MC_CHECK(static_cast<usize>(index) < FixedSize, "Index in array is out of range!");
			return m_Data[static_cast<usize>(index)];
		}

	private:
		ValueType m_Data[ArraySize];
	};

	template<typename T, auto ArraySize>
	using FixedArray = FixedArrayImpl<T, ToArraySize<ArraySize>()>;
}