#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Containers/Array.hpp"
#include "MoonCore/Containers/String.hpp"
#include "MoonCore/Containers/UnorderedMap.hpp"
#include "MoonCore/Memory/Pointers/Scope.hpp"

namespace MC::Filesystem
{
    enum class KnownFolder : u8
    {
        Temp,
        AppData,
        UserHome,
        Documents,
        ExecutableDir,
        WorkingDir
    };

    class Filesystem
    {
    public:
        static Filesystem* Get()
        {
            return s_Instance.Get();
        }

    public:
        static bool Initialize() noexcept;
        static void Shutdown() noexcept;

        static bool CreateDirectory(const MC::StringView& directory) noexcept;
        static bool RemoveDirectory(const MC::StringView& directory) noexcept;
        static bool SetCurrentDirectory(const MC::StringView& directory) noexcept;
        static MC::StringView GetCurrentDirectory() noexcept;

        static bool Move(const MC::StringView& oldLocation, const MC::StringView& newLocation, bool replaceExisting = true, bool yields = false) noexcept;
        static bool Exits(const MC::StringView& path) noexcept;

        static MC::String GetKnownFolder(KnownFolder folder) noexcept;

    private:
        bool SetCurrentDirectoryImpl(const MC::StringView& directory) noexcept;
        MC::StringView GetCurrentDirectoryImpl() const noexcept;

    public:
        virtual ~Filesystem() noexcept = default;
    
    protected:
        virtual bool InitializeImpl() noexcept = 0;
        virtual bool CreateDirectoryPlatformImpl(const MC::StringView& directory) const noexcept = 0;
        virtual bool RemoveDirectoryPlatformImpl(const MC::StringView& directory) const noexcept = 0;
        virtual bool SetCurrentDirectoryPlatformImpl(const MC::StringView& directory) const noexcept = 0;
        virtual MC::String GetCurrentDirectoryPlatformImpl() const noexcept = 0;
        virtual bool MovePlatformImpl(const MC::StringView& oldLocation, 
            const MC::StringView& newLocation, bool replaceExisting, bool yields) const noexcept = 0;
        virtual bool ExistsPlatformImpl(const MC::StringView& path) const noexcept = 0;

        virtual MC::String GetKnownFolderImpl(KnownFolder folder) const noexcept = 0;

    private:
        static Memory::Scope<Filesystem> CreateInstance() noexcept;
    
    private:
        MC::String m_CurrentDirectory;

    private:
        static Memory::Scope<Filesystem> s_Instance;
    };

    class DirectoryStorage
    {
    public:
        static void StoreDirectory(const StringView& name, const StringView& path);
        static MC::StringView GetDirectory(const MC::StringView& directoryName);
    
    private:
        static UnorderedMap<String, String> s_Directories;
    };
}
