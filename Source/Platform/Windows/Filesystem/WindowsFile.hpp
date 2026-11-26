#pragma once
#include "Core.hpp"
#include "Filesystem/File.hpp"
#include "Containers/String.hpp"
#include "Platform/Windows/WindowsInclude.hpp"

namespace MC::Windows::Filesystem
{
    class WindowsFile final : public MC::Filesystem::Details::FileBase
    {
    public:
        WindowsFile(const MC::StringView& path, MC::Filesystem::Enums::ReadMode readMode) noexcept;
        ~WindowsFile() noexcept override;

    public:
        bool Read(MC::String& output, 
            MC::Filesystem::Enums::WordReadMode readMode,
            const StringView& symbols) noexcept override;
        bool Read(Memory::ByteSpan& data) noexcept override;
        
        bool Write(char8 character) noexcept override;
        bool Write(const MC::StringView& str) noexcept override;
        bool Write(const void* input, usize size) noexcept override;
        
        usize Tell() const noexcept override;
        bool Seek(usize position) noexcept override;
        bool SeekToLine(usize line) noexcept override;
        
        bool IsEOF() const noexcept override { return m_EndOfFile; }
        bool IsReadOnly() const noexcept override { return m_FileReadMode == MC::Filesystem::Enums::ReadMode::Read; }
        
        bool Flush() noexcept override;
        
        const MC::Filesystem::FileInfo& GetFileInfo() const noexcept override { return m_FileInfo; }
        MC::Filesystem::Enums::OperationError GetFileLastError() const noexcept override { return m_LastError; }
    
    private:
        void OpenFile(const MC::StringView& path) noexcept;
        constexpr DWORD FileReadModeToDWORD() const noexcept;

    private:
        HANDLE m_File;

    private:
        MC::Filesystem::FileInfo m_FileInfo;
        MC::Filesystem::Enums::ReadMode m_FileReadMode;
        MC::Filesystem::Enums::OperationError m_LastError;
        bool m_EndOfFile;
    };

    constexpr DWORD WindowsFile::FileReadModeToDWORD() const noexcept
    {
        switch (m_FileReadMode)
        {
            case MC::Filesystem::Enums::ReadMode::Write:        return GENERIC_WRITE;
            case MC::Filesystem::Enums::ReadMode::Read:         return GENERIC_READ;
            case MC::Filesystem::Enums::ReadMode::ReadAndWrite: return GENERIC_READ | GENERIC_WRITE;
            case MC::Filesystem::Enums::ReadMode::Unknown:      return 0;
        }
        return 0;
    }
}