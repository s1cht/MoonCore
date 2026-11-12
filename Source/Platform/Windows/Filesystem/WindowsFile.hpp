#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Filesystem/File.hpp"
#include "MoonCore/Containers/String.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Windows::Filesystem
{
    class WindowsFile final : public MC::Filesystem::FileBase
    {
    public:
        WindowsFile(const MC::StringView& path, MC::Filesystem::FileReadMode readMode) noexcept;
        ~WindowsFile() noexcept override;

    public:
        bool Read(MC::String& output, 
            MC::Filesystem::Input::ReadMode readMode,
            const StringView& symbols) noexcept override;
        bool Read(void* data, usize size) noexcept override;
        
        bool Write(char8 character) noexcept override;
        bool Write(const MC::StringView& str) noexcept override;
        bool Write(const void* input, usize size) noexcept override;
        
        usize Tell() const noexcept override;
        bool Seek(usize position) noexcept override;
        bool SeekToLine(usize line) noexcept override;
        
        bool IsEOF() const noexcept override { return m_EndOfFile; }
        bool IsReadOnly() const noexcept override { return m_FileReadMode == MC::Filesystem::FileReadMode::Read; }
        
        bool Flush() noexcept override;
        
        const MC::Filesystem::FileInfo& GetFileInfo() const noexcept override { return m_FileInfo; }
        MC::Filesystem::FileOperationError GetFileLastError() const noexcept override { return m_LastError; }
    
    private:
        void OpenFile(const MC::StringView& path) noexcept;
        constexpr DWORD FileReadModeToDWORD() noexcept;

    private:
        HANDLE m_File;

    private:
        MC::Filesystem::FileInfo m_FileInfo;
        MC::Filesystem::FileReadMode m_FileReadMode;
        MC::Filesystem::FileOperationError m_LastError;
        bool m_EndOfFile;
    };

    constexpr DWORD WindowsFile::FileReadModeToDWORD() noexcept
    {
        switch (m_FileReadMode)
        {
            case MC::Filesystem::FileReadMode::Write:        return GENERIC_WRITE;
            case MC::Filesystem::FileReadMode::Read:         return GENERIC_READ;
            case MC::Filesystem::FileReadMode::ReadAndWrite: return GENERIC_READ | GENERIC_WRITE;
            case MC::Filesystem::FileReadMode::Unknown:      return 0;
        }
        return 0;
    }
}