#pragma once
#include "MoonCore/Memory/Pointers/Scope.hpp"
#include "MoonCore/Memory/Pointers/Reference.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Windows::Concept
{
    template<typename T>
    concept COMObject = IsBaseOf<IUnknown, T> 
    && requires(T* object)
    {
        { object->Release() };
    };
}

namespace MC::Windows::Memory
{
    template <Concept::COMObject T>
    struct COMDeleter
    {
        static void Delete(IUnknown* pointer) noexcept
        {
            pointer->Release();
        }
    };

    template <typename T> using COMScope = MC::Memory::Scope<T, COMDeleter<T>>;
    template <typename T, MC::Memory::ReferenceMode Mode = MC::Memory::DEFAULT_REFERENCE_MODE> using COMReference = MC::Memory::Reference<T, Mode, COMDeleter<T>>;

    template<typename T, typename Callable>
    constexpr COMScope<T> MakeCOMScope(HRESULT& result, Callable&& createFunction) noexcept
    {
        T* object = nullptr;
        result = Invoke(Forward<Callable>(createFunction), &object);
        return COMScope<T>(object);
    }

    template<typename T, typename Callable>
    constexpr COMReference<T> MakeCOMReference(HRESULT& result, Callable&& createFunction) noexcept
    {
        T* object = nullptr;
        result = Invoke(Forward<Callable>(createFunction), &object);
        return COMReference<T>(object);
    }

    template<typename T>
    constexpr COMScope<T> MakeCOMScope(REFCLSID rclsid, LPUNKNOWN pUnkOuter, DWORD dwClsContext, HRESULT& result) noexcept
    {
        T* object = nullptr;
        result = CoCreateInstance(rclsid, pUnkOuter, dwClsContext, IID_PPV_ARGS(&object));
        return COMScope<T>(object);
    }
}
