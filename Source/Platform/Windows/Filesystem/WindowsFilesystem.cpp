#include "WindowsFilesystem.hpp"
#include "MoonCore/Logging/Logger.hpp"
#include "Platform/Windows/WindowsInclude.hpp"
#include "Platform/Windows/WindowsMemory.hpp"

using namespace MC::Logging;

namespace MC::Windows::Filesystem
{
    bool WindowsFilesystem::InitializeImpl() noexcept
    {
        return true;
    }

    bool WindowsFilesystem::CreateDirectoryPlatformImpl(const MC::StringView& directory) const noexcept
    {
        BOOL result = CreateDirectoryW(StringViewToWideString(directory).String(), nullptr);
        if (result != 0) return true;

        DWORD lastError = GetLastError();
        if (lastError == ERROR_ALREADY_EXISTS) return true;

        Logger::Error(Logger::Module::Core,
            "Failed to create directory! Win32 error: %llx", static_cast<i32>(lastError));
        return false;
    }

    bool WindowsFilesystem::RemoveDirectoryPlatformImpl(const MC::StringView& directory) const noexcept
    {
        BOOL result = RemoveDirectoryW(StringViewToWideString(directory).String());
        if (result != 0) return true;
        Logger::Error(Logger::Module::Core,
            "Failed to remove directory! Win32 error: %llx", static_cast<i32>(GetLastError()));
        return false;
    }

    bool WindowsFilesystem::SetCurrentDirectoryPlatformImpl(const MC::StringView& directory) const noexcept
    {
        BOOL result = SetCurrentDirectoryW(StringViewToWideString(directory).String());
        if (result != 0) return true;
        Logger::Error(Logger::Module::Core,
            "Failed to set current directory! Win32 error: %llx", static_cast<i32>(GetLastError()));
        return false;
    }

    MC::String WindowsFilesystem::GetCurrentDirectoryPlatformImpl() const noexcept
    {
        WCHAR path[MAX_PATH];
        DWORD size = GetCurrentDirectoryW(sizeof(path), path);
        if (size != 0) return WideStringViewToString(path);

        Logger::Error(Logger::Module::Core,
            "Failed to get current directory! Win32 error: %llx", static_cast<i32>(GetLastError()));
        return { "", 0 };
    }

    bool WindowsFilesystem::ExistsPlatformImpl(const MC::StringView& path) const noexcept
    {
        return PathFileExists(StringViewToWideString(path).String()) != FALSE;
    }

    bool WindowsFilesystem::MovePlatformImpl(const MC::StringView& oldLocation, const MC::StringView& newLocation, bool replaceExisting, bool yields) const noexcept
    {
        DWORD flags = MOVEFILE_COPY_ALLOWED;
        flags |= replaceExisting ? MOVEFILE_REPLACE_EXISTING : 0;
        flags |= yields ? MOVEFILE_WRITE_THROUGH : 0;

        BOOL result = MoveFileEx(StringViewToWideString(oldLocation).String(),
            StringViewToWideString(newLocation).String(), flags);
        if (result != 0) return true;
        Logger::Error(Logger::Module::Core,
            "Failed to move file/directory! Win32 error: %llx", static_cast<i32>(GetLastError()));
        return false;
    }

    MC::String WindowsFilesystem::GetKnownFolderImpl(MC::Filesystem::KnownFolder folder) const noexcept
    {
        MC::WideString path;
        path.Resize(MAX_PATH);
        if (folder == MC::Filesystem::KnownFolder::Temp)
        {
            GetTempPath(MAX_PATH, path.String());
            return WideStringToString(path);
        }
        if (folder == MC::Filesystem::KnownFolder::ExecutableDir)
        {
            char7* ptr;
            _get_pgmptr(&ptr);
            String result = ptr;
            usize pos = result.FindLast('\\');
            if (pos == String::NoPosition)
                pos = result.FindLast('/');
            return result.Substring(0, pos);
        }
        if (folder == MC::Filesystem::KnownFolder::WorkingDir)
            return GetCurrentDirectoryPlatformImpl();

        KNOWNFOLDERID id;
        switch (folder)
        {
            case MC::Filesystem::KnownFolder::AppData: id = FOLDERID_LocalAppData; break;
            case MC::Filesystem::KnownFolder::UserHome: id = FOLDERID_Profile; break;
            case MC::Filesystem::KnownFolder::Documents: id = FOLDERID_Documents; break;
            case MC::Filesystem::KnownFolder::Temp:
            case MC::Filesystem::KnownFolder::ExecutableDir:
            case MC::Filesystem::KnownFolder::WorkingDir:
                return "";
        }

        PWSTR result;
        SHGetKnownFolderPath(id, 0, nullptr, &result);
        return WideStringToString(result);
    }
}

namespace MC::Filesystem
{
    Memory::Scope<Filesystem> Filesystem::CreateInstance() noexcept
    {
        return MC::Memory::MakeScope<MC::Windows::Filesystem::WindowsFilesystem>();
    }
}
