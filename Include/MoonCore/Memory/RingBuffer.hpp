#pragma once
#include "MoonCore/Types.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Containers/StackArray.hpp"

namespace MC::Memory
{
    template<typename T, usize Count = 0>
    class RingBuffer;

    template<typename T, usize Count>
    class RingBuffer
    {
    public:
        constexpr RingBuffer() noexcept : m_CurrentBuffer(0) {}
        constexpr RingBuffer(MC::StackArray<T, Count>&& buffers) noexcept : m_CurrentBuffer(0), m_Buffers(Move(buffers))
        {
            MC_VALIDATE(m_Buffers.Size() > 0, "Passed empty buffer array");
        }
        virtual ~RingBuffer() = default;

        constexpr void RotateBuffer() noexcept
        {
            m_CurrentBuffer = (m_CurrentBuffer + 1) % BufferCount();
        }
        constexpr T& AcquireBuffer() noexcept
        {
            return m_Buffers[m_CurrentBuffer];
        }
        constexpr const T& AcquireBuffer() const noexcept
        {
            return m_Buffers[m_CurrentBuffer];
        }
        constexpr usize CurrentBuffer() const noexcept { return m_CurrentBuffer; }
        static constexpr usize BufferCount() noexcept { return Count; }

    protected:
        void AssignBuffer(MC::StackArray<T, Count>&& buffers) noexcept
        {
            m_Buffers = Move(buffers);
        }

    protected:
        usize m_CurrentBuffer;
        MC::StackArray<T, Count> m_Buffers;
    };

    template<typename T>
    class RingBuffer<T, 0>
    {
    public:
        RingBuffer() noexcept : m_CurrentBuffer(0), m_Buffers(1) {}
        RingBuffer(MC::Array<T>&& buffers) noexcept : m_CurrentBuffer(0), m_Buffers(Move(buffers))
        {
            MC_VALIDATE(m_Buffers.Size() > 0, "Passed empty buffer array");
        }   

        virtual ~RingBuffer() = default;

        void RotateBuffer() noexcept
        {
            m_CurrentBuffer = (m_CurrentBuffer + 1) % BufferCount();
        }
        T& AcquireBuffer() noexcept
        {
            return m_Buffers[m_CurrentBuffer];
        }
        const T& AcquireBuffer() const noexcept
        {
            return m_Buffers[m_CurrentBuffer];
        }
        usize CurrentBuffer() const noexcept { return m_CurrentBuffer; }
        usize BufferCount() const noexcept { return m_Buffers.Size(); }

    protected:
        void AssignBuffer(MC::Array<T>&& buffers) noexcept
        {
            m_Buffers = Move(buffers);
        }

    protected:
        usize m_CurrentBuffer;
        MC::Array<T> m_Buffers;
    };
}