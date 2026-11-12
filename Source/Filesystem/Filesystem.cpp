#include "MoonCore/Filesystem/Filesystem.hpp"
#include "MoonCore/Logging/Logger.hpp"

using namespace MC::Logging;

namespace MC::Filesystem
{
    Memory::Scope<Filesystem> Filesystem::s_Instance = nullptr;
    UnorderedMap<String, String> DirectoryStorage::s_Directories;

    void DirectoryStorage::StoreDirectory(const StringView& name, const StringView& path)
    {
        if (!Filesystem::Exits(path)) Filesystem::CreateDirectory(path);
        s_Directories.Insert(name.ToString(), path.ToString());
    }

    MC::StringView DirectoryStorage::GetDirectory(const MC::StringView& directoryName)
    {
        if (s_Directories.Contains(directoryName))
            return s_Directories.At(directoryName).Value;
        return "";
    }

    bool Filesystem::Initialize() noexcept
    {
        s_Instance = CreateInstance();
        return s_Instance != nullptr;
    }

    void Filesystem::Shutdown() noexcept
    {
    }

    bool Filesystem::CreateDirectory(const MC::StringView& directory) noexcept
    {
        if (!s_Instance)
        {
            Logger::Debug(Logger::Module::Core, Logger::Severity::Error, "Failed to remove '{}' directory!", directory);
            return false;
        }
        return s_Instance->CreateDirectoryPlatformImpl(directory);
    }
     
    bool Filesystem::RemoveDirectory(const MC::StringView& directory) noexcept
    {
        if (!s_Instance)
        {
            Logger::Debug(Logger::Module::Core, Logger::Severity::Error, "Failed to remove '{}' directory!", directory);
            return false;
        }
        return s_Instance->RemoveDirectoryPlatformImpl(directory);
    }

    bool Filesystem::SetCurrentDirectory(const MC::StringView& directory) noexcept
    {
        if (!s_Instance) return false;
        return s_Instance->SetCurrentDirectoryImpl(directory);
    }

    MC::StringView Filesystem::GetCurrentDirectory() noexcept
    {
        MC_ASSERT(s_Instance, Logging::Logger::Module::Core, "Filesystem can't be accessed!");
        return s_Instance->GetCurrentDirectoryImpl();
    }

    bool Filesystem::Move(const MC::StringView& oldLocation, const MC::StringView& newLocation, bool replaceExisting, bool yields) noexcept
    {
        if (!s_Instance) return false;
        return s_Instance->MovePlatformImpl(oldLocation, newLocation, replaceExisting, yields);
    }

    bool Filesystem::Exits(const MC::StringView& path) noexcept
    {
        if (!s_Instance) return false;
        return s_Instance->ExistsPlatformImpl(path);
    }

    MC::String Filesystem::GetKnownFolder(KnownFolder folder) noexcept
    {
        MC_ASSERT(s_Instance, Logging::Logger::Module::Core, "Filesystem can't be accessed!");
        return s_Instance->GetKnownFolderImpl(folder);
    }

    bool Filesystem::SetCurrentDirectoryImpl(const MC::StringView& directory) noexcept
    {
        bool result = SetCurrentDirectoryPlatformImpl(directory);
        if (!result) return false;
        m_CurrentDirectory = GetCurrentDirectoryPlatformImpl();
        return true;
    }

    MC::StringView Filesystem::GetCurrentDirectoryImpl() const noexcept
    {
        return m_CurrentDirectory;
    }

#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX)
    Memory::Scope<Filesystem> Filesystem::CreateInstance() noexcept { return nullptr; }
#endif
}
