#pragma once
#include "MoonCore/Core.hpp"
#include "MoonCore/Clock/Time.hpp"
#include "MoonCore/Containers/String.hpp"
#include "MoonCore/Memory/Byte.hpp"

namespace MC::Filesystem
{
    namespace Enums
    {
        enum class OperationError : u8
        {
            Success = 0,
            FileNotOpened, FileIsReadOnly,
            ReadFileFailed, WriteFileFailed, SeekFailed, FlushFailed, LockFailed,
            EndOfFile, PartialRead,
            Unknown, UnknownReadMode
        };

        enum class ReadMode : u8
        {
            Read = 0,
            Write,
            ReadAndWrite,
            Unknown
        };

        enum class WordReadMode : u8
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

    namespace Details
    {
        class FileBase
        {
        public:
            virtual ~FileBase() noexcept = default;

            virtual bool Read(MC::String& output, Enums::WordReadMode readMode, const StringView& symbols = TEXT("")) noexcept = 0;
            virtual bool Read(Memory::ByteSpan& data) noexcept = 0;

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
            virtual Enums::OperationError GetFileLastError() const noexcept = 0;

        public:
            static Memory::Scope<FileBase> Create(const MC::StringView& path, Enums::ReadMode readMode) noexcept;
        };
    }

    class COREAPI File
    {
    public:
        File() noexcept;
        explicit File(const MC::StringView& path, 
            Enums::ReadMode readMode = Enums::ReadMode::ReadAndWrite) noexcept;
        File(const File&) = delete;
        File(File&&) noexcept = default;
        ~File() noexcept = default;

    public:
        File& operator=(const File&) = delete;
        File& operator=(File&&) noexcept = default;

    public:
        bool Open(const MC::StringView& path, 
            Enums::ReadMode readMode = Enums::ReadMode::ReadAndWrite) noexcept;
        bool OpenTemporary(const MC::StringView& fileName,
            Enums::ReadMode readMode = Enums::ReadMode::ReadAndWrite) noexcept;

        void Close() noexcept;

    public:
        bool Read(MC::String& output, 
            Enums::WordReadMode readMode = Enums::WordReadMode::Word,
            const StringView& symbols = "\n") const noexcept;
        bool Read(MC::Memory::ByteSpan& data) const noexcept;

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
        Enums::OperationError GetFileLastError() const noexcept;

    private:
        Memory::Scope<Details::FileBase> m_File;
    };
}