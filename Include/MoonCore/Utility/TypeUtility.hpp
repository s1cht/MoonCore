#pragma once
#include "MoonCore/Memory/Hasher.hpp"
#include "MoonCore/Containers/String.hpp"
#include "MoonCore/Utility/Debug.hpp"
#include "MoonCore/Async/Atomic.hpp"

namespace MC::Utility
{
    template<typename Type>
    struct TypeID
    {
        template<typename T>
        constexpr static uptr ID() noexcept
        {
            return Memory::ConstexprHasher<StringView>::Hash(GetType(), 0, 1);
        }
    
    private:
        constexpr static MC::StringView GetType() noexcept
        {
            return MC_FUNCTION;
        }
    };

    namespace Helper
    {
        template<typename ReturnType>
        class TypeIDGeneratorSharedStorage
        {
        public:
            static ReturnType GetNewID() noexcept
            {
                return s_GlobalID.fetch_add(1, std::memory_order_relaxed) + 1;
            }

        private:
            static inline Async::Atomic<ReturnType> s_GlobalID{0};
        };
    }

    template<Concept::Integral ReturnType, typename /* Type */>
    struct TypeIDGenerator
    {
        template<typename> static ReturnType ID() noexcept 
        {
            static ReturnType s_ID = Helper::TypeIDGeneratorSharedStorage<ReturnType>::GetNewID();
            return s_ID; 
        }
    };
}