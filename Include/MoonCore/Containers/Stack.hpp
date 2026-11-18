#pragma once
#include "MoonCore/Containers/Array.hpp"

/**
 * @file Stack.hpp
 * @brief LIFO stack container adapter.
 * @ingroup container_adapters
 */

namespace MC::Concept
{
    /**
     * @concept ContainerEligibleForStack
     * @brief Concept describing container requirements to serve as backing store for `MC::Stack`.
     *
     * Requires `Back()`, `PushBack()`, `EmplaceBack()`, `PopBack()`, `Size()`, and `Empty()`.
     *
     * @tparam Container Backing container type.
     */
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
    /**
     * @class Stack
     * @brief LIFO (Last-In, First-Out) container adapter.
     *
     * `Stack<T, Container>` wraps an underlying sequence container (defaults to `MC::Array<T>`)
     * to provide strict LIFO stack semantics: elements are pushed onto and popped from the top (back) of the container.
     *
     * @tparam T The element type stored in the stack.
     * @tparam Container Underlying template template container. Defaults to `MC::Array`.
     * @tparam ContainerTemplateArgs Additional template parameters for the backing container (such as Allocator).
     *
     * @code
     * MC::Stack<int> s;
     * s.Push(10);
     * s.Push(20);
     * int top = s.Top(); // 20
     * s.Pop();           // Removes 20
     * @endcode
     */
    template <typename T, 
        template <typename, typename...> class Container = Array, 
        typename... ContainerTemplateArgs
    >
        requires Concept::ContainerEligibleForStack<Container<T, ContainerTemplateArgs...>>
    class Stack
    {
    public:
        using ContainerType = Container<T, ContainerTemplateArgs...>;   ///< Backing container type.
        using ValueType = typename ContainerType::ValueType;           ///< Stored element type.
        using RefType = typename ContainerType::RefType;               ///< Reference to element.
        using ConstRefType = typename ContainerType::ConstRefType;     ///< Reference to const element.

    public:
        /** @brief Default constructor. Constructs an empty stack. */
        Stack() = default;

        /**
         * @brief Constructs a stack copying from an existing container.
         * @param container Source container.
         */
        explicit Stack(const ContainerType& container) noexcept : m_Container(container) {}

        /**
         * @brief Constructs a stack moving from an existing container.
         * @param container Source container.
         */
        explicit Stack(ContainerType&& container) noexcept : m_Container(Move(container)) {}

        /** @brief Copy constructor. */
        Stack(const Stack&) noexcept = default;
        /** @brief Move constructor. */
        Stack(Stack&&) noexcept = default;
        /** @brief Destructor. */
        ~Stack() noexcept = default;

    public:
        /** @brief Copy assignment operator. */
        Stack& operator=(const Stack&) noexcept = default;
        /** @brief Move assignment operator. */
        Stack& operator=(Stack&&) noexcept = default;

    public:
        /** @brief Returns number of elements in stack. */
        usize Size() const noexcept { return m_Container.Size(); }
        /** @brief Checks whether stack is empty. */
        bool Empty() const noexcept { return m_Container.Empty(); }

        /**
         * @brief Accesses the top (newest) element.
         * @return Mutable reference to top element.
         */
        RefType Top() noexcept { return m_Container.Back(); }

        /**
         * @brief Accesses the top (newest) element (const overload).
         * @return Const reference to top element.
         */
        ConstRefType Top() const noexcept { return m_Container.Back(); }

        /**
         * @brief Pushes a copy of @p value onto the stack.
         * @param value Element to push.
         */
        void Push(const ValueType& value) noexcept { m_Container.PushBack(value); }

        /**
         * @brief Pushes @p value onto the stack by moving.
         * @param value Element to push.
         */
        void Push(ValueType&& value) noexcept { m_Container.PushBack(Move(value)); }

        /**
         * @brief Constructs an element in-place at the top of the stack.
         * @tparam ArgTypes Constructor argument types.
         * @param args Forwarded constructor arguments.
         * @return Reference to created element.
         */
        template<typename... ArgTypes> RefType Emplace(ArgTypes&&... args) noexcept { return m_Container.EmplaceBack(Forward<ArgTypes>(args)...); }

        /**
         * @brief Removes the top element from the stack.
         */
        void Pop() noexcept { m_Container.PopBack(); }

    private:
        ContainerType m_Container; ///< Underlying container storage.
    };
}
