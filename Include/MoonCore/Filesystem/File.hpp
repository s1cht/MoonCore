#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Clock/Time.hpp"
#include "MoonCore/Containers/String.hpp"

namespace MC::Filesystem
{
    enum class FileOperationError : u8
	{
		Success = 0,
		FileNotOpened, FileIsReadOnly,
		ReadFileFailed, WriteFileFailed, SeekFailed, FlushFailed, LockFailed,
        EndOfFile, PartialRead,
        Unknown, UnknownReadMode
	};

    enum class FileReadMode : u8
    {
        Read = 0,
        Write,
        ReadAndWrite,
        Unknown
    };

    namespace Input
    {
        enum class ReadMode
        {
            Word = 0,
            Line,
            UntilSymbol,
            All
        };
    }

    struct FileInfo
	{
		MC::String Name;
		usize Size;
		MC::Clock::Timepoint CreationTime = Clock::Timepoint();
		MC::Clock::Timepoint LastWriteTime = Clock::Timepoint();
	};

    class FileBase
    {
    public:
        virtual ~FileBase() noexcept = default;
        
        virtual bool Read(MC::String& output, Input::ReadMode readMode, const StringView& symbols = TEXT("")) noexcept = 0;
        virtual bool Read(void* data, usize size) noexcept = 0;

        virtual bool Write(char8 character) noexcept = 0;
        virtual bool Write(const MC::StringView& str) noexcept = 0;
        virtual bool Write(const void* input, usize size) noexcept = 0;

        virtual usize Tell() const noexcept = 0;
        virtual bool Seek(usize position) noexcept = 0;
        virtual bool SeekToLine(usize line) noexcept = 0;

        virtual bool IsEOF() const noexcept = 0;
        virtual bool IsReadOnly() const noexcept = 0;
	
        virtual bool Flush() noexcept = 0;

        virtual const FileInfo& GetFileInfo() const noexcept = 0;
        virtual FileOperationError GetFileLastError() const noexcept = 0;

    public:
        static Memory::Scope<FileBase> Create(const MC::StringView& path, FileReadMode readMode) noexcept;
    };

    class File
    {
    public:
        File() noexcept;
        explicit File(const MC::StringView& path, 
            FileReadMode readMode = FileReadMode::ReadAndWrite) noexcept;
        File(const File&) = delete;
        ~File() noexcept = default;

    public:
        File& operator=(const File&) = delete;

    public:
        bool Open(const MC::StringView& path, 
            FileReadMode readMode = FileReadMode::ReadAndWrite) noexcept;
        bool OpenTemporary(const MC::StringView& fileName,
            FileReadMode readMode = FileReadMode::ReadAndWrite) noexcept;

        void Close() noexcept;

    public:
        bool Read(MC::String& output, 
            Input::ReadMode readMode = Input::ReadMode::Word, 
            const StringView& symbols = "\n") const noexcept;
        
        bool Write(char8 character) const noexcept;
        bool Write(const MC::StringView& output) const noexcept;
        bool Write(const void * const input, usize size) const noexcept;

        usize Tell() const noexcept;
        bool Seek(usize position) const noexcept;
        bool SeekToLine(usize line) const noexcept;

        bool IsEOF() const noexcept;
        bool IsReadOnly() const noexcept;
        bool IsOpened() const noexcept { return m_File; }

        bool Flush() const noexcept;

        usize GetFileSize() const noexcept;
        const FileInfo& GetFileInfo() const noexcept;
        FileOperationError GetFileLastError() const noexcept;

    private:
        Memory::Scope<FileBase> m_File;
    };
}