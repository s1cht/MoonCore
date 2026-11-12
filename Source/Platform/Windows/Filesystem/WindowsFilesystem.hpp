#pragma once
#include "MoonCore/Filesystem/Filesystem.hpp"

namespace MC::Windows::Filesystem
{
    class WindowsFilesystem final : public MC::Filesystem::Filesystem
    {
    protected:
        bool InitializeImpl() noexcept override;
        bool CreateDirectoryPlatformImpl(const MC::StringView& directory) const noexcept override;
        bool RemoveDirectoryPlatformImpl(const MC::StringView& directory) const noexcept override;
        bool SetCurrentDirectoryPlatformImpl(const MC::StringView& directory) const noexcept override;
        MC::String GetCurrentDirectoryPlatformImpl() const noexcept override;
        bool ExistsPlatformImpl(const MC::StringView& path) const noexcept override;
        bool MovePlatformImpl(const MC::StringView& oldLocation, const MC::StringView& newLocation, bool replaceExisting, bool yields) const noexcept override;
    
        MC::String GetKnownFolderImpl(MC::Filesystem::KnownFolder folder) const noexcept override;
    };
}
