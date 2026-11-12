#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Memory/Pointers/Reference.hpp"

namespace MC
{
	template <class BaseClass>
    class ClassInterface : Memory::EnableReferenceFromThis<BaseClass>
    {
	public:
		template<typename T>
		MC_NODISCARD constexpr T* As() const noexcept
		{
			static_assert(!IsSame<T, BaseClass> && IsBaseOf<BaseClass, T>,
				"T must be derived from RenderObject, but not the BaseClass itself!");
			return static_cast<T*>(this);
		}
		template<typename T>
		MC_NODISCARD MC::Memory::Reference<T> AsReference() const noexcept
		{
			static_assert(!IsSame<T, BaseClass> && IsBaseOf<BaseClass, T>,
				"T must be derived from BaseClass, but not the BaseClass itself!");
			return MC::Memory::StaticPointerCast<T>(this->ReferenceFromThis());
		}
		template<typename T>
		MC_NODISCARD MC::Memory::WeakReference<T> AsWeak() const noexcept
		{
			static_assert(!IsSame<T, BaseClass> && IsBaseOf<BaseClass, T>,
				"T must be derived from BaseClass, but not the BaseClass itself!");
			return this->WeakFromThis();
		}
	};
}