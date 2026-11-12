#pragma once
#include "TypeTraits.hpp"

namespace MC::Concept
{
	template<typename A, typename B> concept SameAs = IsSame<A, B>&& IsSame<B, A>;
	template<typename T> concept Moveable = IsObject<T> && IsMoveConstructible<T> && IsAssignable<T&, T>&& IsSwappable<T>;

	namespace Helper
	{
		template<typename T> using Referenced = T&;
	}

	template<typename T> concept CanReference = requires { typename Helper::Referenced<T>; };

	template<typename T, typename U = T> concept HasAddition = requires(T a, U b) { a + b; };
	template<typename T, typename U = T> concept HasSubtraction = requires(T a, U b) { a - b; };
	template<typename T, typename U = T> concept HasMultiplication = requires(T a, U b) { a* b; };
	template<typename T, typename U = T> concept HasDivision = requires(T a, U b) { a / b; };
	template<typename T, typename U = T> concept HasLessCompareOperation = requires(T a, U b) { a < b; };
	template<typename T, typename U = T> concept HasLessEqualCompareOperation = requires(T a, U b) { a <= b; };
	template<typename T, typename U = T> concept HasGreaterCompareOperation = requires(T a, U b) { a > b; };
	template<typename T, typename U = T> concept HasGreaterEqualCompareOperation = requires(T a, U b) { a >= b; };
	template<typename T, typename U = T> concept HasEqualCompare = requires(T a, U b) { a == b; };

	template<typename T> concept FloatingPoint = IsFloatingPoint<T>;
	template<typename T> concept Integral = IsIntegral<T> ;
	template<typename T> concept SignedIntegral = !IsSame<bool, T> && IsIntegral<T> && IsSigned<T>;
	template<typename T> concept UnsignedIntegral = !IsSame<bool, T> && IsIntegral<T> && IsUnsigned<T>;

	template<typename T> concept Number = IsArithmetic<T> && !IsSame<bool, T>;
	template<typename T> concept Character = IsCharacter<T>;

	template<typename T> concept ScopedEnum = IsScopedEnum<T>;
	template<typename T> concept CountedScopedEnum = ScopedEnum<T> && requires { T::Count; };

	template<class Base, class Derived>
	concept IsDerived = !IsSame<Base, Derived>&& IsBaseOf<Base, Derived>;

	template<typename Function, typename ReturnValue, typename... ArgTypes>
	concept IsNoexcept = (IsSame<ReturnValue, void> && IsNothrowInvocable<Function, ArgTypes...>) ||
		IsNothrowInvocableR<Function, ReturnValue, ArgTypes...>;

	template <typename Function, typename ReturnType>
	concept FunctionReturns = requires(Function function)
	{
		{ function() } -> SameAs<ReturnType>;
	};

	template <typename T>
	concept DefaultInitiazable = IsConstructible<T> 
    && requires 
    {
		T{};
		::new (static_cast<void*>(nullptr)) T;
	};

	template<typename T, typename... ArgTypes>
	concept HasContructor = requires(ArgTypes&&... args)
	{
		{ T(Forward<ArgTypes>(args)...) } -> SameAs<T>;
	};
}	