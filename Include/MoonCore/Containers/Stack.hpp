#pragma once
#include "Array.hpp"

namespace MC::Concept
{
    template <class Container>
    concept ContainerEligibleForStack = IsNothrowDefaultConstructible<Container>
    && requires
    {
        typename Container::ValueType;
        typename Container::RefType;
        typename Container::ConstRefType;
    }
    && requires(Container container, const typename Container::ValueType& lValue, typename Container::ValueType&& rValue)
    {
        { container.Back() } -> ConvertibleTo<typename Container::RefType>;
        { const_cast<const Container&>(container).Back() } -> ConvertibleTo<typename Container::ConstRefType>;
        { container.PushBack(lValue) };
        { container.PushBack(rValue) };
        { container.EmplaceBack() };
        { container.PopBack() };
        { container.Size() } -> ConvertibleTo<usize>;
        { container.Empty() } -> ConvertibleTo<bool>;
    };
}

namespace MC
{
    template <typename T, 
        template <typename, typename...> class Container = Array, 
        typename... ContainerTemplateArgs
    >
        requires Concept::ContainerEligibleForStack<Container<T, ContainerTemplateArgs...>>
    class Stack
    {
    public:
        using ContainerType = Container<T, ContainerTemplateArgs...>;
        using ValueType = typename ContainerType::ValueType;
        using RefType = typename ContainerType::RefType;
        using ConstRefType = typename ContainerType::ConstRefType;

    public:
        Stack() = default;
        explicit Stack(const ContainerType& container) noexcept : m_Container(container) {}
        explicit Stack(ContainerType&& container) noexcept : m_Container(Move(container)) {}
        Stack(const Stack&) noexcept = default;
        Stack(Stack&&) noexcept = default;
        ~Stack() noexcept = default;

    public:
        Stack& operator=(const Stack&) noexcept = default;
        Stack& operator=(Stack&&) noexcept = default;

    public:
        usize Size() const noexcept { return m_Container.Size(); }
        bool Empty() const noexcept { return m_Container.Empty(); }

        RefType Top() noexcept { return m_Container.Back(); }
        ConstRefType Top() const noexcept { return m_Container.Back(); }

        void Push(const ValueType& value) noexcept { m_Container.PushBack(value); }
        void Push(ValueType&& value) noexcept { m_Container.PushBack(Move(value)); }
        template<typename... ArgTypes> RefType Emplace(ArgTypes&&... args) noexcept { return m_Container.EmplaceBack(Forward<ArgTypes>(args)...); }
        void Pop() noexcept { m_Container.PopBack(); }

    private:
        ContainerType m_Container;
    };
}