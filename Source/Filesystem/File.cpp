#include "Filesystem/File.hpp"
#include "Filesystem/Filesystem.hpp"
#include "Logging/Logger.hpp"

namespace MC::Filesystem
{
    const FileInfo DUMMY_FILE_INFO = {};

    File::File() noexcept : m_File(nullptr) {}

    File::File(const MC::StringView& path, Enums::ReadMode readMode) noexcept
        : m_File(Details::FileBase::Create(path, readMode))
    {
        if (!m_File) return;
        if (m_File->GetFileLastError() == Enums::OperationError::Success) return;
        m_File.Reset();
    }

    bool File::Open(const MC::StringView& path, Enums::ReadMode readMode) noexcept
    {
        m_File = Details::FileBase::Create(path, readMode);
        if (m_File) return true;
        return false;
    }

    bool File::OpenTemporary(const MC::StringView& fileName, Enums::ReadMode readMode) noexcept
    {
        m_File = Details::FileBase::Create(DirectoryStorage::GetDirectory("Temp").ToString() + fileName, readMode);
        if (m_File) return true;
        return false;
    }

    void File::Close() noexcept
    {
        m_File.Reset();
    }

    bool File::Read(MC::String& output, Enums::WordReadMode readMode, const StringView& symbols) const noexcept
    {
        if (!m_File) return false;
        return m_File->Read(output, readMode, symbols);
    }

    bool File::Read(MC::Memory::ByteSpan& data) const noexcept
    {
        if (!m_File) return false;
        return m_File->Read(data);
    }

    bool File::Write(char8 character) const noexcept
    {
        if (!m_File) return false;
        return m_File->Write(character);
    }

    bool File::Write(const MC::StringView& output) const noexcept
    {
        if (!m_File) return false;
        return m_File->Write(output);
    }

    bool File::Write(const void * const input, usize size) const noexcept
    {
        if (!m_File) return false;
        return m_File->Write(input, size);
    }

    usize File::Tell() const noexcept
    {
        if (!m_File) return false;
        return m_File->Tell();
    }

    bool File::Seek(usize position) const noexcept
    {
        if (!m_File) return false;
        return m_File->Seek(position);
    }

    bool File::SeekToLine(usize line) const noexcept
    {
        if (!m_File) return false;
        return m_File->SeekToLine(line);
    }

    bool File::IsEOF() const noexcept
    {
        if (!m_File) return true;
        return m_File->IsEOF();
    }

    bool File::IsReadOnly() const noexcept
    {
        if (!m_File) return true;
        return m_File->IsReadOnly();
    }

    bool File::Flush() const noexcept
    {
        if (!m_File) return false;
        return m_File->Flush();
    }

    usize File::GetFileSize() const noexcept
    {
        if (!m_File) return 0;
        return m_File->GetFileInfo().Size;
    }

    const FileInfo& File::GetFileInfo() const noexcept
    {
        if (!m_File) return DUMMY_FILE_INFO;
        return m_File->GetFileInfo();
    }

    Enums::OperationError File::GetFileLastError() const noexcept
    {
        if (!m_File) return Enums::OperationError::FileNotOpened;
        return m_File->GetFileLastError();
    }

#if !defined(PLATFORM_WINDOWS) && !defined(PLATFORM_LINUX)
    Memory::Scope<FileBase> FileBase::Create(const MC::StringView&, FileReadMode) noexcept { return nullptr; }
#endif
}