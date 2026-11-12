#pragma once
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Utility/Iterator.hpp"

namespace MC
{
	template<Concept::Character CharacterType, bool IsConst = false>
	class StringIterator
	{
	public:
		using ValueType	= CharacterType;
		using DiffType	= iptr;
		using PtrType	= Conditional<IsConst, const ValueType*, ValueType*>;
		using RefType	= Conditional<IsConst, const ValueType&, ValueType&>;
		using Category	= Iterator::Tags::RandomAccess;

	public:
		constexpr StringIterator() noexcept : m_Ptr(nullptr) {}
		constexpr StringIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		constexpr StringIterator& operator+=(usize index) { m_Ptr += index; return *this; }
		constexpr StringIterator& operator-=(usize index) { m_Ptr -= index; return *this; }

		constexpr StringIterator operator+(usize index) const { return StringIterator(m_Ptr + index); }
		constexpr StringIterator operator-(usize index) const { return StringIterator(m_Ptr - index); }

		constexpr StringIterator& operator++() { ++m_Ptr; return *this; }
		constexpr StringIterator operator++(int) { StringIterator temp = *this; ++m_Ptr; return temp; }

		constexpr StringIterator& operator--() { --m_Ptr; return (*this); }
		constexpr StringIterator operator--(int) { StringIterator temp = *this; --m_Ptr; return temp; }

		constexpr DiffType operator-(const StringIterator& it) const noexcept { return m_Ptr - it.m_Ptr; }

		constexpr bool operator==(const StringIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		constexpr bool operator!=(const StringIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }
		constexpr bool operator<(const StringIterator& it) const noexcept { return m_Ptr < it.m_Ptr; }
		constexpr bool operator>(const StringIterator& it) const noexcept { return m_Ptr > it.m_Ptr; }

		constexpr RefType operator[](usize index) const { return *(m_Ptr + index); }
		constexpr PtrType operator->() const { return m_Ptr; }
		constexpr RefType operator*() const { return *m_Ptr; }
		constexpr PtrType Ptr() const { return m_Ptr; }

	private:
		PtrType m_Ptr;
	};

	template <Concept::Character CharacterType, bool IsConst = false>
	class CodepointIterator;

	template <bool IsConst>
	class CodepointIterator<char8, IsConst>
	{
	public:
		using ValueType = char32;
		using DiffType = iptr;
		using PtrType = Conditional<IsConst, const char8*, char8*>;
		using RefType = Conditional<IsConst, const char32, char32>;
		using Category = Iterator::Tags::Bidirectional;

	public:
		constexpr CodepointIterator() noexcept : m_Ptr(nullptr) {}
		constexpr CodepointIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		constexpr CodepointIterator& operator+=(usize count)
		{
			while (count--) ++(*this);
			return *this;
		}
		constexpr CodepointIterator& operator-=(usize count)
		{
			while (count--) --(*this);
			return *this;
		}

		constexpr CodepointIterator operator+(usize count) const { CodepointIterator tmp = *this; tmp += count; return tmp; }
		constexpr CodepointIterator operator-(usize count) const { CodepointIterator tmp = *this; tmp -= count; return tmp; }

		constexpr CodepointIterator& operator++()
		{
			u8 lead = static_cast<u8>(*m_Ptr);
			MC_CHECK((lead & 0xC0) != 0x80, "Iterator points to a UTF-8 continuation byte");
			if (lead < 0x80) m_Ptr += 1;
			else if (lead < 0xE0) m_Ptr += 2;
			else if (lead < 0xF0) m_Ptr += 3;
			else m_Ptr += 4;
			return *this;
		}
		constexpr CodepointIterator& operator--()
		{
			do --m_Ptr; while ((static_cast<u8>(*m_Ptr) & 0xC0) == 0x80);
			return *this;
		}

		constexpr CodepointIterator operator++(int) { CodepointIterator tmp = *this; ++(*this); return tmp; }
		constexpr CodepointIterator operator--(int) { CodepointIterator tmp = *this; --(*this); return tmp; }

		constexpr DiffType operator-(const CodepointIterator & other) const
		{
			DiffType count = 0;
			CodepointIterator tmp = (m_Ptr > other.m_Ptr) ? other : *this;
			CodepointIterator end = (m_Ptr > other.m_Ptr) ? *this : other;
			while (tmp != end) { ++tmp; ++count; }
			return (m_Ptr > other.m_Ptr) ? count : -count;
		}

		constexpr bool operator==(const CodepointIterator & it) const noexcept { return m_Ptr == it.m_Ptr; }
		constexpr bool operator!=(const CodepointIterator & it) const noexcept { return m_Ptr != it.m_Ptr; }
		constexpr bool operator<(const CodepointIterator & it) const noexcept { return m_Ptr < it.m_Ptr; }
		constexpr bool operator>(const CodepointIterator & it) const noexcept { return m_Ptr > it.m_Ptr; }

		constexpr RefType operator*() const { return Decode(); }
		constexpr PtrType operator->() const { return m_Ptr; }
		constexpr RefType operator[](usize index) const { return *(*this + index); }
		constexpr const char8* Ptr() const { return m_Ptr; }

	private:
		constexpr char32 Decode() const
		{
			u8 lead = static_cast<u8>(*m_Ptr);
			if (lead < 0x80) return lead;
			if (lead < 0xE0) return (lead & 0x1F) << 6 | (m_Ptr[1] & 0x3F);
			if (lead < 0xF0) return (lead & 0x0F) << 12 | (m_Ptr[1] & 0x3F) << 6 | (m_Ptr[2] & 0x3F);
			return (lead & 0x07) << 18 | (m_Ptr[1] & 0x3F) << 12 | (m_Ptr[2] & 0x3F) << 6 | (m_Ptr[3] & 0x3F);
		}

	private:
		PtrType m_Ptr;
	};

	template <bool IsConst>
	class CodepointIterator<wchar, IsConst>
	{
	public:
		using ValueType = char32;
		using DiffType = iptr;
		using PtrType = Conditional<IsConst, const wchar*, wchar*>;
		using RefType = Conditional<IsConst, const char32, char32>;
		using Category = Iterator::Tags::Bidirectional;

	public:
		constexpr CodepointIterator() noexcept : m_Ptr(nullptr) {}
		constexpr CodepointIterator(PtrType ptr) noexcept : m_Ptr(ptr) {}

	public:
		constexpr CodepointIterator& operator+=(usize count)
		{
			while (count--) ++(*this);
			return *this;
		}
		constexpr CodepointIterator& operator-=(usize count)
		{
			while (count--) --(*this);
			return *this;
		}

		constexpr CodepointIterator operator+(usize count) const { CodepointIterator tmp = *this; tmp += count; return tmp; }
		constexpr CodepointIterator operator-(usize count) const { CodepointIterator tmp = *this; tmp -= count; return tmp; }

		constexpr CodepointIterator& operator++()
		{
#ifdef PLATFORM_WINDOWS
			u16 unit = static_cast<u16>(*m_Ptr);
			if ((unit & 0xFC00) == 0xD800) m_Ptr += 2;
			else m_Ptr += 1;
#else
			m_Ptr++;
#endif
		    return *this;
		}
		constexpr CodepointIterator& operator--()
		{
#ifdef PLATFORM_WINDOWS
			do --m_Ptr; while ((static_cast<u16>(*m_Ptr) & 0xFC00) == 0xDC00);
#else
			--m_Ptr;
#endif
		    return *this;
		}

		constexpr CodepointIterator operator++(int) { CodepointIterator tmp = *this; ++(*this); return tmp; }
		constexpr CodepointIterator operator--(int) { CodepointIterator tmp = *this; --(*this); return tmp; }

		constexpr DiffType operator-(const CodepointIterator& other) const
		{
			DiffType count = 0;
			CodepointIterator tmp = (m_Ptr > other.m_Ptr) ? other : *this;
			CodepointIterator end = (m_Ptr > other.m_Ptr) ? *this : other;
			while (tmp != end) { ++tmp; ++count; }
			return (m_Ptr > other.m_Ptr) ? count : -count;
		}

		constexpr bool operator==(const CodepointIterator& it) const noexcept { return m_Ptr == it.m_Ptr; }
		constexpr bool operator!=(const CodepointIterator& it) const noexcept { return m_Ptr != it.m_Ptr; }
		constexpr bool operator<(const CodepointIterator& it) const noexcept { return m_Ptr < it.m_Ptr; }
		constexpr bool operator>(const CodepointIterator& it) const noexcept { return m_Ptr > it.m_Ptr; }

		constexpr RefType operator*() const { return Decode(); }
		constexpr PtrType operator->() const { return m_Ptr; }
		constexpr RefType operator[](usize index) const { return *(*this + index); }
		constexpr const wchar* Ptr() const { return m_Ptr; }

	private:
		constexpr char32 Decode() const
		{
#ifdef PLATFORM_WINDOWS
			u16 unit = static_cast<u16>(*m_Ptr);
			if ((unit & 0xFC00) != 0xD800) return static_cast<char32>(unit);
			u16 high = unit;
			u16 low = static_cast<u16>(*(m_Ptr + 1));
			return 0x10000 + ((static_cast<char32>(high & 0x3FF) << 10) | (low & 0x3FF));
#else
			return static_cast<char32>(*m_Ptr);
#endif
		}

	private:
		PtrType m_Ptr;
	};
}
