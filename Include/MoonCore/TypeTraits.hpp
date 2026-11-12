#pragma once
#include <intrin.h>
#include "Types.hpp"
#include "Macros.hpp"

namespace MC
{
	using NullptrT = decltype(nullptr);
	template<typename...> using VoidT = void;
		
	template<typename T> constexpr bool IsEnum = __is_enum(T);
	template<typename T> constexpr bool IsUnion = __is_union(T);
	template<typename T> constexpr bool IsClass = __is_class(T);
	template<typename T> constexpr bool IsTrivial = __is_trivial(T);

	template<typename> constexpr bool IsArray = false;
	template<typename T, usize N> constexpr bool IsArray<T[N]> = true;
	template<typename T> constexpr bool IsArray<T[]> = true;
	template<typename> constexpr bool IsBoundedArray = false;
	template<typename T, usize N> constexpr bool IsBoundedArray<T[N]> = true;
	template<typename> constexpr bool IsUnboundedArray = false;
	template<typename T> constexpr bool IsUnboundedArray<T[]> = true;
	template<typename> constexpr bool IsPointer = false;
	template<typename T> constexpr bool IsPointer<T*> = true;

	template<typename> constexpr bool IsReference = false;
	template<typename T> constexpr bool IsReference<T&> = true;
	template<typename T> constexpr bool IsReference<T&&> = true;

	template<typename> constexpr bool IsConst = false;
	template<typename T> constexpr bool IsConst<const T> = true;

	template<typename> constexpr bool IsVolatile = false;
	template<typename T> constexpr bool IsVolatile<volatile T> = true;

	template<typename T> constexpr bool IsFunction = !IsConst<const T> && !IsReference<T>;

    namespace Details
    {
		struct EmptyStruct { constexpr EmptyStruct() noexcept {} };

		template <typename T>
		struct ExtractTemplateH;

		template <template <typename...> class TemplateName, typename T>
		struct ExtractTemplateH<TemplateName<T>>
		{
			template <typename NewType>
			using Template = TemplateName<NewType>;

			using ExtractedValue = T;
		};

		template <template<typename> class Base, typename To> struct RebindH { using Value = Base<To>; };
		template <class Extractable, typename To> struct ExtractRebindH { using Value = typename ExtractTemplateH<Extractable>::template Template<To>; };

		template<usize Index, typename... Types> struct TypeFromTypelistH { static_assert(Index < sizeof...(Types), "Typelist index out of range"); };
		template<usize Index, typename T, typename... Rest> struct TypeFromTypelistH<Index, T, Rest...> { using Value = typename TypeFromTypelistH<Index - 1, Rest...>::Value; };
		template<typename T, typename... Rest> struct TypeFromTypelistH<0, T, Rest...> { using Value = T; };

		template<typename Callable>
		struct TemplateCallableH;

		template<typename ReturnType, typename... ArgTypes>
		struct TemplateCallableH<ReturnType(ArgTypes...)>
		{
			using Value = ReturnType(ArgTypes...);
		};

		template<typename T> struct RemoveConstH { using Type = T; };
		template<typename T> struct RemoveConstH<const T> { using Type = T; };

		template<typename T> struct RemoveVolatileH { using Type = T; };
		template<typename T> struct RemoveVolatileH<volatile T> { using Type = T; };

        template<typename T> struct RemoveReferenceH { using Type = T; };
        template<typename T> struct RemoveReferenceH<T&> { using Type = T; };
        template<typename T> struct RemoveReferenceH<T&&> { using Type = T; };

        template<typename T> struct RemoveCVH { using Type = T; };
        template<typename T> struct RemoveCVH<const T> { using Type = T; };
        template<typename T> struct RemoveCVH<volatile T> { using Type = T; };
        template<typename T> struct RemoveCVH<const volatile T> { using Type = T; };

		template<typename T> struct RemovePointerH { using Type = T; };
		template<typename T> struct RemovePointerH<T*> { using Type = T; };

		template<class T> struct RemoveExtentH { using Type = T; };
		template<class T> struct RemoveExtentH<T[]> { using Type = T; };
		template<class T, usize N> struct RemoveExtentH<T[N]> { using Type = T; };

		template<class T> struct RemoveAllExtentsH { using Type = T; };
		template<class T> struct RemoveAllExtentsH<T[]> { using Type = RemoveAllExtentsH<T>::Type; };
		template<class T, usize N> struct RemoveAllExtentsH<T[N]> { using Type = RemoveAllExtentsH<T>::Type; };

		template<typename T, typename = void> 
        struct AddReferenceH
		{
		    using LValue = T;
			using RValue = T;
		};
		template<typename T> 
        struct AddReferenceH<T, VoidT<T&>>
		{
			using LValue = T&;
			using RValue = T&&;
		};

		template<typename T, typename = void>
		struct AddPointerH
		{
			using Type = T;
		};
		template<typename T>
		struct AddPointerH<T, VoidT<typename RemoveReferenceH<T>::Type>>
		{
			using Type = RemoveReferenceH<T>::Type*;
		};

		template<bool Test, typename TrueResult, typename FalseResult> struct ConditionalH { using Type = TrueResult; };
		template<typename TrueResult, typename FalseResult> struct ConditionalH<false, TrueResult, FalseResult> { using Type = FalseResult; };

		template<typename T> struct IsMemberObjectH
		{
			static constexpr bool Value = false;
		};
		template<typename Member, typename Class> struct IsMemberObjectH<Member Class::*>
		{
			static constexpr bool Value = !IsFunction<Member>;
			using ClassType = Class;
			using MemberType = Member;
		};

		template<typename T> struct IsMemberFunctionH
		{
			static constexpr bool Value = false;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...)						   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) noexcept				   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const					   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) volatile				   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const volatile		   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const noexcept		   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) volatile noexcept		   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const volatile noexcept  >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) &						   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) & noexcept			   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const&				   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) volatile&				   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const volatile&		   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const& noexcept		   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) volatile& noexcept	   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const volatile& noexcept >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) &&					   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) && noexcept		       >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const&&				   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
        template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) volatile&&			   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const volatile&&		   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const&& noexcept		   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) volatile&& noexcept	   >
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};
		template<typename Return, typename Class, typename... Arguments> struct IsMemberFunctionH<Return (Class::*)(Arguments...) const volatile&& noexcept>
		{
			static constexpr bool Value = true;
			using ClassType = Class;
			using ReturnType = Return;
		};

		template <usize IntSize> struct MakeSignH {};
		template<> struct MakeSignH<1>
		{
		    using Signed = i8;
		    using Unsigned = u8;
		};
		template<> struct MakeSignH<2>
		{
			using Signed = i16;
			using Unsigned = u16;
		};
		template<> struct MakeSignH<4>
		{
			using Signed = i32;
			using Unsigned = u32;
		};
		template<> struct MakeSignH<8>
		{
			using Signed = i64;
			using Unsigned = u64;
		};

		template<typename> struct ExtractMemberObjectTypesH {};
		template<typename ClassType, typename MemberType> 
        struct ExtractMemberObjectTypesH<MemberType ClassType::*>
		{
			using Class = ClassType;
			using Member = MemberType;
		};

		template<typename ReturnType, typename ClassType, typename... Other>
		struct ExtractMemberObjectTypesH<ReturnType (ClassType::*)(Other..., ...)>
		{
			using Class = ClassType;
		};
		
		void TryConvertEnum(...);
		void TryConvertEnum(int) = delete;

		template<typename Enum>
		concept IsScopedEnumH = IsEnum<Enum> && requires { TryConvertEnum(Enum{}); };

        template<typename T, bool = IsEnum<T>> struct UnderlyingTypeH { using Type = __underlying_type(T); };
		template<typename T> struct UnderlyingTypeH<T, false> {};
    }

    MC_NODISCARD constexpr bool IsConstantEvaluated() noexcept
    {
        return __builtin_is_constant_evaluated();
    }
	
	template<bool Test, typename TrueResult, typename FalseResult>
	using Conditional = typename Details::ConditionalH<Test, TrueResult, FalseResult>::Type;

    template <typename T> using RemoveReference   = typename Details::RemoveReferenceH<T>::Type;
    template <typename T> using RemoveConst		  = typename Details::RemoveConstH<T>::Type;
    template <typename T> using RemoveVolatile	  = typename Details::RemoveVolatileH<T>::Type;
    template <typename T> using RemoveCV		  = typename Details::RemoveCVH<T>::Type;
    template <typename T> using RemoveCVReference = RemoveCV<RemoveReference<T>>;
    template <typename T> using RemovePointer	  = typename Details::RemovePointerH<T>::Type;
    template <typename T> using RemoveExtent	  = typename Details::RemoveExtentH<T>::Type;
    template <typename T> using RemoveAllExtents  = typename Details::RemoveAllExtentsH<T>::Type;

	template<typename To, typename From>
	MC_NODISCARD constexpr To BitCast(const From& value) noexcept
	{
		return __builtin_bit_cast(To, value);
	}

	template<typename T> constexpr bool IsMemberFunctionPointer = Details::IsMemberFunctionH<RemoveCV<T>>::Value;
	template<typename T> constexpr bool IsMemberObjectPointer = Details::IsMemberObjectH<RemoveCV<T>>::Value;

	template<typename, typename> constexpr bool IsSame = false;
	template<typename T> constexpr bool IsSame<T, T> = true; 

    template<typename> constexpr bool IsLValueReference = false;
    template<typename T> constexpr bool IsLValueReference<T&> = true;

	template<typename> constexpr bool IsRValueReference = false;
	template<typename T> constexpr bool IsRValueReference<T&&> = true;

	template<typename T> using AddLValueReference = typename Details::AddReferenceH<T>::LValue;
	template<typename T> using AddRValueReference = typename Details::AddReferenceH<T>::RValue;

	template<typename T> using ExtractMemberType = typename Details::ExtractMemberObjectTypesH<T>::Member;
	template<typename T> using ExtractClassType = typename Details::ExtractMemberObjectTypesH<T>::Class;

	template<typename T> using AddPointer = typename Details::AddPointerH<T>::Type;

	template<typename T> constexpr bool IsTriviallyCopyable = __is_trivially_copyable(T);
	template<typename T> constexpr bool IsStandardLayout = __is_standard_layout(T);
	template<typename T> constexpr bool HasUniqueObjectRepresentations = __has_unique_object_representations(T);
	template<typename T> constexpr bool IsEmpty = __is_empty(T);
	template<typename T> constexpr bool IsPolymorphic = __is_polymorphic(T);
	template<typename T> constexpr bool IsAbstract = __is_abstract(T);
	template<typename T> constexpr bool IsFinal = __is_final(T);
	template<typename T> constexpr bool IsAggregate = __is_aggregate(T);
	template<typename T> constexpr bool IsImplicitLifetime = __builtin_is_implicit_lifetime(T);

	template<typename T> constexpr bool IsDestructible								   = __is_destructible(T);
	template<typename T> constexpr bool IsTriviallyDestructible						   = __is_trivially_destructible(T);
	template<typename T> constexpr bool IsNothrowDestructible						   = __is_nothrow_destructible(T);
	template<typename T, typename... ArgTypes> constexpr bool IsConstructible		   = __is_constructible(T, ArgTypes...);
	template<typename T, typename... ArgTypes> constexpr bool IsTriviallyConstructible = __is_trivially_constructible(T, ArgTypes...);
	template<typename T, typename... ArgTypes> constexpr bool IsNothrowConstructible   = __is_nothrow_constructible(T, ArgTypes...);
	template<typename T> constexpr bool IsDefaultConstructible						   = __is_constructible(T);
	template<typename T> constexpr bool IsTriviallyDefaultConstructible				   = __is_trivially_constructible(T);
	template<typename T> constexpr bool IsNothrowDefaultConstructible				   = __is_nothrow_constructible(T);
	template<typename T> constexpr bool IsCopyConstructible							   = __is_constructible(T, AddLValueReference<T>);
	template<typename T> constexpr bool IsTriviallyCopyConstructible				   = __is_trivially_constructible(T, AddLValueReference<T>);
	template<typename T> constexpr bool IsNothrowCopyConstructible					   = __is_nothrow_constructible(T, AddLValueReference<T>);
	template<typename T> constexpr bool IsMoveConstructible							   = __is_constructible(T, AddRValueReference<T>);
	template<typename T> constexpr bool IsTriviallyMoveConstructible				   = __is_trivially_constructible(T, AddRValueReference<T>);
	template<typename T> constexpr bool IsNothrowMoveConstructible					   = __is_nothrow_constructible(T, AddRValueReference<T>);

	template<typename To, typename From> constexpr bool IsAssignable			= __is_assignable(To, From);
	template<typename To, typename From> constexpr bool IsTriviallyAssignable	= __is_trivially_assignable(To, From);
	template<typename To, typename From> constexpr bool IsNothrowAssignable		= __is_nothrow_assignable(To, From);
	template<typename T> constexpr bool IsCopyAssignable						= __is_assignable(AddLValueReference<T>, AddLValueReference<const T>);
	template<typename T> constexpr bool IsTriviallyCopyAssignable				= __is_trivially_assignable(AddLValueReference<T>, AddLValueReference<const T>);
	template<typename T> constexpr bool IsNothrowCopyAssignable					= __is_nothrow_assignable(AddLValueReference<T>, AddLValueReference<const T>);
	template<typename T> constexpr bool IsMoveAssignable						= __is_assignable(AddLValueReference<T>, AddRValueReference<T>);
	template<typename T> constexpr bool IsTriviallyMoveAssignable				= __is_trivially_assignable(AddLValueReference<T>, AddRValueReference<T>);
	template<typename T> constexpr bool IsNothrowMoveAssignable					= __is_nothrow_assignable(AddLValueReference<T>, AddRValueReference<T>);

	template<typename T> constexpr bool HasVirtualDestructor = __has_virtual_destructor(T);

	template<typename T> constexpr usize AlignmentOf = alignof(T);

	template<typename T> AddRValueReference<T> DeclareValue() noexcept;

	namespace Concept
	{
		template <class From, class To>
	    concept ConvertibleTo = __is_convertible_to(From, To) && requires { static_cast<To>(DeclareValue<From>()); };
	}

	template<typename Base, typename Derived> constexpr bool IsBaseOf = __is_base_of(Base, Derived);
	template<typename From, typename To> constexpr bool IsConvertible = Concept::ConvertibleTo<From, To>;
	template<typename From, typename To> constexpr bool IsNothrowConvertible = IsConvertible<From, To>&& requires(From from) { { To(from) } noexcept; };
	template<typename T, typename U> constexpr bool IsLayoutCompatible = __is_layout_compatible(T, U);
	template<typename Base, typename Derived> constexpr bool IsPointerInterconvertibleBaseOf = __is_pointer_interconvertible_base_of(Base, Derived);

	template <typename T>
	MC_NODISCARD constexpr T&& Forward(RemoveReference<T>& arg) noexcept
	{
		return static_cast<T&&>(arg);
	}

	template <typename T>
	MC_NODISCARD constexpr T&& Forward(RemoveReference<T>&& arg) noexcept
	{
		static_assert(!IsLValueReference<T>, "Bad Forward call");
		return static_cast<T&&>(arg);
	}

	template <typename T>
	MC_NODISCARD constexpr RemoveReference<T>&& Move(T&& arg) noexcept
	{
		return static_cast<RemoveReference<T>&&>(arg);
	}

	template<typename A>
	constexpr void Swap(A& left, A& right) 
        noexcept(IsNothrowAssignable<AddLValueReference<A>, AddRValueReference<A>>)
	{
		A temp = Move(left);
		left   = Move(right);
		right  = Move(temp);
	}

	template<typename T> constexpr bool IsVoid = IsSame<void, T>;
	template<typename T> constexpr bool IsNullptr = IsSame<NullptrT, T>;
	template<typename T> constexpr bool IsMemberObject = Details::IsMemberObjectH<T>::Value;
	template<typename T> constexpr bool IsMemberFunction = Details::IsMemberFunctionH<T>::Value;
	template<typename T> constexpr bool IsScopedEnum = Details::IsScopedEnumH<T>;
	
	template<typename T, typename... Types> constexpr bool IsAnySame = (IsSame<RemoveCV<T>, RemoveCV<Types>> || ...);
	template<typename T> constexpr bool IsIntegral = IsAnySame <T, bool, 
        char, signed char, unsigned char, 
        char8_t, char16_t, char32_t,
		short, signed short, unsigned short, 
        int, signed int, unsigned int, 
        long, signed long, unsigned long, 
        long long, signed long long, unsigned long long>;
	template<typename T> constexpr bool IsFloatingPoint = IsAnySame<T, float, double, long double>;
	template<typename T> constexpr bool IsCharacter = IsAnySame<T, char7, char8, char16, char32, wchar>;

	namespace Details
	{
		template <typename T, bool = IsIntegral<T>>
		struct TypeSign 
	    {
			static constexpr bool Signed = static_cast<RemoveCV<T>>(-1) < static_cast<RemoveCV<T>>(0);
			static constexpr bool Unsigned = !Signed;
		};

		template <typename T>
		struct TypeSign<T, false>
		{
			static constexpr bool Signed = IsFloatingPoint<T>;
			static constexpr bool Unsigned = false;
		};

		template<typename T> 
	        requires IsIntegral<T>
		using MakeSigned = MakeSignH<sizeof(T)>::Signed;
		template<typename T>
			requires IsIntegral<T>
		using MakeUnsigned = MakeSignH<sizeof(T)>::Unsigned;
	
		template<typename A> constexpr void Swap(A&, A&) noexcept(IsNothrowAssignable<AddLValueReference<A>, AddRValueReference<A>>);

		template<typename A, typename B, typename = void> constexpr bool IsSwappableWithDetails = false;
		template<typename A, typename B> constexpr bool IsSwappableWithDetails<A, B, VoidT<decltype(Details::Swap(DeclareValue<A>(), DeclareValue<B>()))>> = true;

		template<typename A, typename B, typename = void, bool = false> constexpr bool IsIsNothowSwappableWithDetails = false;
		template<typename A, typename B> constexpr bool IsIsNothowSwappableWithDetails<A, B, VoidT<decltype(Details::Swap(DeclareValue<A>(), DeclareValue<B>()))>,
	        noexcept(Details::Swap(DeclareValue<A>(), DeclareValue<B>()))> = true;

		template<typename A, typename B = A> constexpr bool _IsSwappableWith = IsSwappableWithDetails<A, B>&& IsSwappableWithDetails<B, A>;
		template<typename A, typename B = A> constexpr bool _IsNothrowSwappableWith = IsSwappableWithDetails<A, B>&& IsSwappableWithDetails<B, A>;
		template<typename A, typename B = A> constexpr bool IsSwappableWith = _IsSwappableWith<AddLValueReference<A>, AddLValueReference<B>>;
		template<typename A, typename B = A> constexpr bool IsNothrowSwappableWith = _IsNothrowSwappableWith<AddLValueReference<A>, AddLValueReference<B>>;
	}

    template<typename T> constexpr bool IsSigned = Details::TypeSign<T>::Signed;
	template<typename T> constexpr bool IsUnsigned = Details::TypeSign<T>::Unsigned;

	template<typename T> using MakeSigned = typename Details::MakeSigned<T>;
	template<typename T> using MakeUnsigned = typename Details::MakeUnsigned<T>;

	template<typename T> constexpr bool IsArithmetic = IsFloatingPoint<T> || IsIntegral<T>;
	template<typename T> constexpr bool IsFundamental = IsArithmetic<T> || IsVoid<T> || IsNullptr<T>;
	template<typename T> constexpr bool IsMemberPointer = IsMemberFunction<T> || IsMemberObject<T>;
	template<typename T> constexpr bool IsScalar = IsArithmetic<T> || IsEnum<T> || IsPointer<T> || IsMemberPointer<T> || IsNullptr<T>;
	template<typename T> constexpr bool IsObject = !IsFunction<T> && !IsReference<T> && !IsVoid<T>;
	template<typename T> constexpr bool IsCompound = !IsFundamental<T>;

	template<typename A, typename B> constexpr bool IsSwappableWith = Details::IsSwappableWith<A, B>;
	template<typename A> constexpr bool IsSwappable = Details::IsSwappableWith<A>;
	template<typename A, typename B> constexpr bool IsNothrowSwappableWith = Details::IsNothrowSwappableWith<A, B>;
	template<typename A, typename B> constexpr bool IsNothrowSwappable = Details::IsNothrowSwappableWith<A>;

	template<bool Test, typename T = void> using EnableIf = Conditional<Test, T, Details::EmptyStruct>;

	template<typename T> using UnderlyingType = Details::UnderlyingTypeH<T>::Type;

	template <class Extractable> using ExtractTemplate = Details::ExtractTemplateH<Extractable>::ExtractedTemplate;
	template <class Extractable> using ExtractTemplateValues = Details::ExtractTemplateH<Extractable>::ExtractedValue;

	template <template<typename> class Base, typename To> using Rebind = Details::RebindH<Base, To>::Value;
	template <class Extractable, typename To> using ExtractRebind = Details::ExtractRebindH<Extractable, To>::Value;

	template<usize Index, typename... Rest> using TypeFromTypelist = Details::TypeFromTypelistH<Index, Rest...>::Value;

	template<typename... Types>
	struct Typelist
	{
		static constexpr usize Size = (usize{ 0 } + ... + sizeof(Types));
		static constexpr usize Count = sizeof...(Types);
		template<usize Index> using Type = Details::TypeFromTypelistH<Index, Types...>::Value;
	};

	template<typename, typename> constexpr bool IsSameTemplate = false;
	template<template<typename> class TemplateName, typename A, typename B> constexpr bool IsSameTemplate<TemplateName<A>, TemplateName<B>> = true;

	template<typename Callable>
	using TemplateCallable = Details::TemplateCallableH<Callable>::Value;

	template<class Enum>
	constexpr UnderlyingType<Enum> ToUnderlying(Enum enumerator) noexcept { return static_cast<UnderlyingType<Enum>>(enumerator); }

	template <typename T>
	MC_NODISCARD constexpr T* AddressOf(T& value) noexcept
	{
		return __builtin_addressof(value);
	}

	namespace Details
	{
	    template<typename T>
		struct Decay
	    {
			using Cleared = RemoveReference<T>;
			using Type = Conditional<IsArray<Cleared>, 
	            AddPointer<RemoveExtent<Cleared>>, 
	            Conditional<IsFunction<Cleared>, 
	                AddPointer<Cleared>, 
	                RemoveCV<Cleared>
	            >
	        >;
	    };
	}


	template<typename T> using Decay = typename Details::Decay<T>::Type;

	namespace Details
	{
		namespace InvokeMethods
		{
			enum class InvokerMethod
			{
				Functor,
				FunctionPointerObject,
				FunctionPointerPointer,
				MemberPointerObject,
				MemberPointerPointer,
			};

		    struct InvokeFunctor
		    {
				static constexpr InvokerMethod Method = InvokerMethod::Functor;

				template<typename Callable, typename... ArgTypes>
		        static constexpr auto Call(Callable&& callable, ArgTypes&&... args) 
		            noexcept(noexcept(Forward<Callable>(callable)(Forward<ArgTypes>(args)...)))
					-> decltype(Forward<Callable>(callable)(Forward<ArgTypes>(args)...))
		        {
					return static_cast<Callable&&>(callable)(static_cast<ArgTypes&&>(args)...);
				}
		    };

			struct InvokeFunctionPointerObject 
		    {
				static constexpr InvokerMethod Method = InvokerMethod::FunctionPointerObject;

				template <typename DecayedCallable, typename ObjectType, typename...OtherArgTypes>
				static constexpr auto Call(DecayedCallable methodPointer, ObjectType&& object, OtherArgTypes&&... args)
					noexcept(noexcept((static_cast<ObjectType&&>(object).*methodPointer)(static_cast<OtherArgTypes&&>(args)...)))
					-> decltype((static_cast<ObjectType&&>(object).*methodPointer)(static_cast<OtherArgTypes&&>(args)...))
			    {
					return (static_cast<ObjectType&&>(object).*methodPointer)(static_cast<OtherArgTypes&&>(args)...);
				}
			};

			struct InvokeFunctionPointerPointer
		    {
				static constexpr InvokerMethod Method = InvokerMethod::FunctionPointerPointer;

				template <typename DecayedCallable, typename ObjectType, typename...OtherArgTypes>
				static constexpr auto Call(DecayedCallable methodPointer, ObjectType&& object, OtherArgTypes&&... args)
					noexcept(noexcept((*(static_cast<ObjectType&&>(object)).*methodPointer)(static_cast<OtherArgTypes&&>(args)...)))
					-> decltype((*(static_cast<ObjectType&&>(object)).*methodPointer)(static_cast<OtherArgTypes&&>(args)...))
			    {
					return (*(static_cast<ObjectType&&>(object)).*methodPointer)(static_cast<OtherArgTypes&&>(args)...);
				}
			};

			struct InvokeMemberPointerObject
			{
				static constexpr InvokerMethod Method = InvokerMethod::MemberPointerObject;

				template <typename DecayedCallable, typename Member>
				static constexpr auto Call(DecayedCallable methodPointer, Member&& arg)
					noexcept(noexcept(static_cast<Member&&>(arg).*methodPointer))
					-> decltype(static_cast<Member&&>(arg).*methodPointer)
				{
					return (static_cast<Member&&>(arg).*methodPointer);
				}
			};

			struct InvokeMemberPointerPointer
			{
				static constexpr InvokerMethod Method = InvokerMethod::MemberPointerPointer;

				template <typename DecayedCallable, typename Member>
				static constexpr auto Call(DecayedCallable methodPointer, Member&& arg)
					noexcept(noexcept(*(static_cast<Member&&>(arg)).*methodPointer))
					-> decltype(*(static_cast<Member&&>(arg)).*methodPointer)
				{
					return (*(static_cast<Member&&>(arg)).*methodPointer);
				}
			};
		}

		template<typename Callable, typename ObjectType, typename ClearCallable = RemoveCVReference<Callable>,
	        bool IsMemberObjectPointer = IsMemberObjectPointer<ClearCallable>,
	        bool IsMemberFunctionPointer = IsMemberFunctionPointer<ClearCallable>>
		struct Invoker;

		template<typename Callable, typename ObjectType, typename ClearCallable>
	    struct Invoker<Callable, ObjectType, ClearCallable, false, true>
		    : Conditional<
			    IsSame<typename IsMemberFunctionH<ClearCallable>::ClassType, RemoveCVReference<ObjectType>> 
	                || IsBaseOf<typename IsMemberFunctionH<ClearCallable>::ClassType, RemoveCVReference<ObjectType>>,
	              InvokeMethods::InvokeFunctionPointerObject,
	              InvokeMethods::InvokeFunctionPointerPointer>
		{};

		template<typename Callable, typename ObjectType, typename ClearCallable>
		struct Invoker<Callable, ObjectType, ClearCallable, true, false>
			: Conditional<
			    IsSame<typename IsMemberObjectH<ClearCallable>::ClassType, RemoveCVReference<ObjectType>>
	                || IsBaseOf<typename IsMemberObjectH<ClearCallable>::ClassType, RemoveCVReference<ObjectType>>,
			InvokeMethods::InvokeMemberPointerObject,
			InvokeMethods::InvokeMemberPointerPointer>
	    {};

		template<typename Callable, typename ObjectType, typename ClearCallable>
		struct Invoker<Callable, ObjectType, ClearCallable, false, false> : InvokeMethods::InvokeFunctor {};
	}

	template<typename Callable>
	static constexpr auto Invoke(Callable&& callable) 
        noexcept(noexcept(Forward<Callable>(callable)()))
		-> decltype(Forward<Callable>(callable)())
    {
		return Forward<Callable>(callable)();
	}

	template<typename Callable, typename ObjectType, typename... OtherArgTypes>
	static constexpr auto Invoke(Callable&& callable, ObjectType&& object, OtherArgTypes&&... otherArgs)
		noexcept(noexcept(Details::Invoker<Callable, ObjectType>::Call(
			static_cast<Callable&&>(callable), static_cast<ObjectType&&>(object), static_cast<OtherArgTypes&&>(otherArgs)...)))
		-> decltype(Details::Invoker<Callable, ObjectType>::Call(
			static_cast<Callable&&>(callable), static_cast<ObjectType&&>(object), static_cast<OtherArgTypes&&>(otherArgs)...))
	{
		if constexpr (Details::Invoker<Callable, ObjectType>::Method == Details::InvokeMethods::InvokerMethod::Functor)
			return static_cast<Callable&&>(callable)(static_cast<ObjectType&&>(object), static_cast<OtherArgTypes&&>(otherArgs)...);
		else if constexpr (Details::Invoker<Callable, ObjectType>::Method == Details::InvokeMethods::InvokerMethod::FunctionPointerObject)
			return (static_cast<ObjectType&&>(object).*callable)(static_cast<OtherArgTypes&&>(otherArgs)...);
		else if constexpr (Details::Invoker<Callable, ObjectType>::Method == Details::InvokeMethods::InvokerMethod::FunctionPointerPointer) 
			return ((*static_cast<ObjectType&&>(object)).*callable)(static_cast<OtherArgTypes&&>(otherArgs)...);
		else if constexpr (Details::Invoker<Callable, ObjectType>::Method == Details::InvokeMethods::InvokerMethod::MemberPointerObject) 
			return static_cast<ObjectType&&>(object).*callable;
		else 
			return (*static_cast<ObjectType&&>(object)).*callable;
	}

	template<typename Callable, typename... Args> using InvokeResult = decltype(Invoke(DeclareValue<Callable>(), DeclareValue<Args>()...));

	template<typename Callable, typename... Args> constexpr bool IsInvocable						  = requires { {Invoke(DeclareValue<Callable>(), DeclareValue<Args>()...) }; };
	template<typename Return, typename Callable, typename... Args> constexpr bool IsInvocableR		  = requires { { Invoke(DeclareValue<Callable>(), DeclareValue<Args>()...) } -> Concept::ConvertibleTo<Return>; };
	template<typename Callable, typename... Args> constexpr bool IsNothrowInvocable					  = requires { { Invoke(DeclareValue<Callable>(), DeclareValue<Args>()...) } noexcept; };
	template<typename Return, typename Callable, typename... Args> constexpr bool IsNothrowInvocableR = requires { { Invoke(DeclareValue<Callable>(), DeclareValue<Args>()...) } noexcept -> Concept::ConvertibleTo<Return>; };

    template <class T, T... Values>
	    requires IsIntegral<T>
    struct IntSequence 
    {
		using value_type = T;

		MC_NODISCARD static constexpr usize size() noexcept { return sizeof...(Values); }
	};
}