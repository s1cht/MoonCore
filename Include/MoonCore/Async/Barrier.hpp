#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "Atomic.hpp"

namespace MC::Async
{
    namespace Token
    {
        constexpr u64 BARRIER_INVALID_TOKEN = 0b0;
        constexpr u64 BARRIER_PHASE_MASK = 0b1;
        constexpr u64 BARRIER_VALUE_OFFSET = 1;
        constexpr u64 BARRIER_VALUE_MASK = ~BARRIER_PHASE_MASK;
        constexpr u64 BARRIER_MAX = BARRIER_VALUE_MASK >> BARRIER_VALUE_OFFSET;
    }

    struct EmptyCompletionFunction
    {
        void operator()() const noexcept {}
    };

    template <typename CompletionFunction = EmptyCompletionFunction, u64 Expected = 0>
        requires IsNothrowInvocable<CompletionFunction&> && (Expected <= Token::BARRIER_MAX)
    class Barrier;

    namespace Helper
    {
        template <typename CompletionFunction, u64 Expected>
        class ArrivalTokenImpl
        {
        public:
            ArrivalTokenImpl(const ArrivalTokenImpl&) = delete;
            constexpr ArrivalTokenImpl(ArrivalTokenImpl&& o) noexcept : m_Token(o.m_Token) { o.m_Token = Token::BARRIER_INVALID_TOKEN; }
            ~ArrivalTokenImpl() noexcept = default;
            
            ArrivalTokenImpl& operator=(const ArrivalTokenImpl&) = delete;
            constexpr ArrivalTokenImpl& operator=(ArrivalTokenImpl&& o) noexcept { m_Token = o.m_Token; o.m_Token = Token::BARRIER_INVALID_TOKEN; return *this; }

            constexpr u64 Phase() const noexcept { return m_Token & Token::BARRIER_PHASE_MASK; }
            constexpr u64 Address() const noexcept { return m_Token & Token::BARRIER_VALUE_MASK; }

        private:
            constexpr ArrivalTokenImpl(u64 token) noexcept : m_Token(token) {}
            friend class Barrier<CompletionFunction, Expected>;
            u64 m_Token;
        };

        template<u64 ExpectedValue>
        struct BarrierData
        {
            constexpr BarrierData(const u64) noexcept : Current(ExpectedValue << Token::BARRIER_VALUE_OFFSET) {}
            static constexpr u64 Total() noexcept { return ExpectedValue << Token::BARRIER_VALUE_OFFSET; }

            AtomicU64 Current;
        };

        template<>
        struct BarrierData<0>
        {
            constexpr BarrierData(const u64 expected) noexcept : m_Total(expected), Current(expected) {}
            u64 Total() const noexcept { return m_Total.load(std::memory_order_acquire); }

            AtomicU64 m_Total;
            AtomicU64 Current;
        };
    }

    template <typename CompletionFunction, u64 Expected>
        requires IsNothrowInvocable<CompletionFunction&> && (Expected <= Token::BARRIER_MAX)
    class Barrier
    {
    public:
        using BarrierData = Helper::BarrierData<Expected>;
        using ArrivalToken = Helper::ArrivalTokenImpl<CompletionFunction, Expected>;

    public:
        explicit Barrier(CompletionFunction function = CompletionFunction())
            : m_Data(0), m_Function(Move(function))
        {
            static_assert(Expected != 0, "Barrier(CompletionFunction) construct is only supported on a compile-count barrier");
        }

        explicit Barrier(const u64 expected, CompletionFunction function = CompletionFunction())
            : m_Data(expected << Token::BARRIER_VALUE_OFFSET), m_Function(Move(function))
        {}

        Barrier(const Barrier&) = delete;

    public:
        Barrier& operator=(const Barrier&) = delete;

    public:
        u64 Max() const noexcept { return m_Data.Total() >> Token::BARRIER_VALUE_OFFSET; }

    public:
        ArrivalToken Arrive(u64 n = 1) noexcept
        {
            MC_CHECK(n > 0 && n <= Max(), "Barrier: n is expected to be > 0 and <= Max()");
            n <<= Token::BARRIER_VALUE_OFFSET;
            MC_CHECK(n <= m_Data.Current.load(std::memory_order_relaxed), "Barrier corruption! n > current expected count");
            u64 current = m_Data.Current.fetch_sub(n) - n;

            if ((current & Token::BARRIER_VALUE_MASK) == 0)
                Completed(current);

            return ArrivalToken((current & Token::BARRIER_PHASE_MASK) | reinterpret_cast<u64>(this));
        }

        void Wait(ArrivalToken&& token) noexcept
        {
            MC_CHECK(token.Address() == reinterpret_cast<u64>(this), "Barrier: used different barrier's token");
            const u64 phase = token.Phase();
            token.m_Token = Token::BARRIER_INVALID_TOKEN;

            for (;;)
            {
                const u64 current = m_Data.Current.load(std::memory_order_acquire);
                if ((current & Token::BARRIER_PHASE_MASK) != phase)
                    break;
                m_Data.Current.wait(current, std::memory_order_relaxed);
            }
        }

        void ArriveAndWait(u64 n = 1) noexcept
        {
            MC_CHECK(n > 0 && n <= Max(), "Barrier: n is expected to be > 0 and <= Max()");
            n <<= Token::BARRIER_VALUE_OFFSET;
            MC_CHECK(n <= m_Data.Current.load(std::memory_order_relaxed), "Barrier corruption! n > current expected count");
            u64 current = m_Data.Current.fetch_sub(n) - n;
            const u64 phase = current & Token::BARRIER_PHASE_MASK;

            if ((current & Token::BARRIER_VALUE_MASK) == 0)
            {
                Completed(current);
                return;
            }

            for (;;)
            {
                current = m_Data.Current.load(std::memory_order_acquire);
                if ((current & Token::BARRIER_PHASE_MASK) != phase)
                    break;
                m_Data.Current.wait(current, std::memory_order_relaxed);
            }
        }

        void ArriveAndDrop(u64 n = 1) noexcept
        {
            static_assert(Expected == 0, "ArriveAndDrop is only supported on a runtime-count barrier");
            m_Data.m_Total.fetch_sub(n << Token::BARRIER_VALUE_OFFSET, std::memory_order_relaxed);
            static_cast<void>(Arrive(n));
        }
        
    private:
        void Completed(u64 current) noexcept
        {
            const u64 newCurrent = m_Data.Total() | ((current + 1) & Token::BARRIER_PHASE_MASK);
            m_Function();
            m_Data.Current.store(newCurrent, std::memory_order_release);
            m_Data.Current.notify_all();
        }

    private:
        BarrierData m_Data;
        CompletionFunction m_Function;
    };
}