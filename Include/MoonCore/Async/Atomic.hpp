#pragma once
#include "MoonCore/Types.hpp"
#include <atomic>

namespace MC::Async
{
    template <typename T> using Atomic = std::atomic<T>;

	typedef Atomic<bool>			AtomicBool;
	typedef Atomic<char7>			AtomicChar7;
	typedef Atomic<wchar>			AtomicWchar;
	typedef Atomic<char8>			AtomicChar8;
	typedef Atomic<char16>			AtomicChar16;
	typedef Atomic<char32>			AtomicChar32;

	typedef Atomic<i8>				AtomicI8;
	typedef Atomic<i16>				AtomicI16;
	typedef Atomic<i32>				AtomicI32;
	typedef Atomic<i64>				AtomicI64;

	typedef Atomic<u8>				AtomicU8;
	typedef Atomic<u16>				AtomicU16;
	typedef Atomic<u32>				AtomicU32;
	typedef Atomic<u64>				AtomicU64;

	typedef Atomic<usize>			AtomicUsize;
	typedef Atomic<isize>			AtomicIsize;
	typedef Atomic<uptr>			AtomicUptr;
	typedef Atomic<iptr>			AtomicIptr;
}