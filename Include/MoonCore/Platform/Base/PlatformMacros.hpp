#pragma once

#if defined(__aarch64__) || defined(_M_AMD64) || defined(__amd64__) || defined(__amd64) || defined(__x86_64__) || defined(__x86_64)
    #define ARCHITECTURE_X64
#else
    #error "Unsupported processor architecture"
#endif

#ifdef _MSC_VER
    #define COMPILER_MSVC
#elif defined(__GNUC__)
    #define COMPILER_GNUC
#elifdef __clang__ 
    #define COMPILER_CLANG
#else
    #error "Unsupported compiler"
#endif

#ifdef _WIN64
    #define PLATFORM_WINDOWS
#elif defined(__linux__)
    #define PLATFORM_UNIX
    #ifdef defined(__ANDROID__)
        #error "Unsupported system"
    #else
        #define PLATFORM_LINUX
    #endif
#else
    #define PLATFORM_UNSUPPORTED
    #error "Unsupported system"
#endif