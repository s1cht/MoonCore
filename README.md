# MoonCore

A modern, high-performance C++23 foundation library engineered for game engines, simulation systems, and performance-critical applications.

---

## Features

- **Containers**: Cache-friendly, exception-free (`noexcept`), allocator-aware data structures including `Array`, `Deque`, `FixedArray`, `StackArray`, `Bitset`, `BinaryHeap`, `Map`, `Set`, `Multimap`, `Multiset`, `UnorderedMap`, `UnorderedSet`, `Queue`, `Stack`, and high-performance UTF-8 / UTF-16 `String` and `StringView`.
- **Memory Management**: Custom allocators (`HeapAllocator`, `StackAllocator`, `PoolAllocator`), memory tracking, and cache-conscious utilities.
- **Multithreading & Async**: Low-overhead thread pools, task systems, synchronization primitives, and custom wait strategies.
- **Diagnostics & Profiling**: High-precision timers, performance profiling, assertion/validation macros, and integrated structured logging.
- **Filesystem & Utility**: Cross-platform path handling, binary serialization, compile-time type traits, and C++23 concepts.

---

## Documentation

Comprehensive API documentation is generated using **Doxygen** with modern responsive styling.

### Generating Documentation Locally

**Prerequisites:**
- [Doxygen](https://www.doxygen.nl/) (version 1.9.0 or higher recommended)

**On Windows:**
Run the batch script:
```bat
ProjectGeneration\GenerateDocs.bat
```
or run directly:
```bat
doxygen Doxyfile
```

**On Linux / macOS:**
```bash
./scripts/generate_docs.sh
```
or:
```bash
doxygen Doxyfile
```

Once generated, view the interactive documentation by opening `docs/html/index.html` in any web browser.

---

## Building MoonCore

MoonCore uses [Premake5](https://premake.github.io/) to generate project files and build configurations.

### Windows (Visual Studio)
Run one of the generation scripts located in `ProjectGeneration/`:
- `GenerateProjectsShared.bat`: Generates solution for shared library build (`.dll`).
- `GenerateProjectsStatic.bat`: Generates solution for static library build (`.lib`).

Or manually invoke premake:
```bat
premake5 --file=solution.lua --link=static vs2026
```

### Supported Platforms
- Windows (x64) - Fully supported
- Linux (x64) - Planned / in progress

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
