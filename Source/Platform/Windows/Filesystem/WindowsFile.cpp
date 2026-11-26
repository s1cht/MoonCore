#include "WindowsFile.hpp"
#include "Logging/Logger.hpp"

using namespace MC::Filesystem;
using namespace MC::Logging;

constexpr MC::Clock::Timepoint WindowsFileTimeToTimepoint(FILETIME fileTime) noexcept
{
    u64 ticks = (static_cast<u64>(fileTime.dwHighDateTime) << 32) | static_cast<u64>(fileTime.dwLowDateTime);
    constexpr u64 ticksPerSecond = 10000000ULL;
    constexpr u64 epochDiffSeconds = 11644473600ULL;
    return { ticks / ticksPerSecond - epochDiffSeconds };
}

namespace MC::Windows::Filesystem
{
    WindowsFile::WindowsFile(const StringView& path, Enums::ReadMode readMode) noexcept
        : m_File(nullptr), m_FileInfo({}), m_FileReadMode(readMode), 
        m_LastError(Enums::OperationError::Unknown), m_EndOfFile(true)
    {
        OpenFile(path);
    }

    WindowsFile::~WindowsFile() noexcept
    {
        if (!m_File) return;
		Flush();
        CloseHandle(m_File);
        m_File = nullptr;
    }

    bool WindowsFile::Read(String& output, Enums::WordReadMode readMode,
        const StringView& symbols) noexcept
    {
        char8 byte = 0;
		DWORD bytesRead;
		bool readingStarted = false;

		output.Clear();

        if (readMode == Enums::WordReadMode::All)
        {
            u64 lastPos = Tell();
		    Seek(0);
		    usize size = m_FileInfo.Size;
		    output.Resize(size);

            Memory::ByteSpan buffer(output);
            bool result = Read(buffer);
            Seek(lastPos);
            return result;
        }

		while (!m_EndOfFile)
		{
			if (Tell() >= m_FileInfo.Size)
			{
				m_EndOfFile = true;
				break;
			}

			if (!ReadFile(m_File, &byte, 1, &bytesRead, nullptr))
			{
                Logging::Logger::Error(Logging::Logger::Module::Core,
                    "Failed to read file! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
				m_LastError = Enums::OperationError::ReadFileFailed;
				m_EndOfFile = true;
				break;
			}

			if (bytesRead == 0)
			{
				m_EndOfFile = true;
				break;
			}

			if (readMode == Enums::WordReadMode::Line && byte == '\n') break;
			if (readMode == Enums::WordReadMode::Word)
			{
				if (byte == '\n' || byte == '\r' || byte == '\0' || byte == ' ')
				{
					if (readingStarted) break;
				    continue;
				}
			    readingStarted = true;
			}
            if (readMode == Enums::WordReadMode::UntilSymbol)
            {
                if (symbols.Find(byte) != StringView::NoPosition)
                {
                    if (readingStarted) break;
                    continue;
                }
                readingStarted = true;
            }

		    output += byte;
		}
		return !output.Empty();
    }

    bool WindowsFile::Read(Memory::ByteSpan& data) noexcept
    {
        DWORD bytesRead;

        if (!ReadFile(m_File, data.Data(), data.Size(), &bytesRead, nullptr))
        {
            Logging::Logger::Debug(Logging::Logger::Module::Core, Logger::Severity::Warn,
                "Failed to read binary data from file! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
            m_LastError = Enums::OperationError::ReadFileFailed;
            return false;
        }

        if (bytesRead != data.Size())
        {
            Logging::Logger::Debug(Logging::Logger::Module::Core, Logger::Severity::Warn,
                "Partially read the file! Requested size: {:d}, got from file: {:d}", data.Size(), bytesRead);
            m_LastError = Enums::OperationError::PartialRead;
            m_EndOfFile = true;
            return false;
        }

        m_LastError = Enums::OperationError::Success;
        return true;
    }

    bool WindowsFile::Write(char8 character) noexcept
    {
        if (m_FileReadMode == Enums::ReadMode::Read)
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Trying to write into read-only file!");
            m_LastError = Enums::OperationError::FileIsReadOnly;
            return false;
        }

        DWORD bytesWritten;
        if (!WriteFile(m_File, &character, 1, &bytesWritten, nullptr))
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Failed to write in file! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
            m_LastError = Enums::OperationError::WriteFileFailed;
            return false;
        }

        m_LastError = Enums::OperationError::Success;
        return true;
    }

    bool WindowsFile::Write(const StringView& str) noexcept
    {
        if (m_FileReadMode == Enums::ReadMode::Read)
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Trying to write into read-only file!");
            m_LastError = Enums::OperationError::FileIsReadOnly;
            return false;
        }

        DWORD bytesWritten;
        if (!WriteFile(m_File, str.String(), static_cast<DWORD>(str.Size()), &bytesWritten, nullptr))
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Failed to write in file! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
            m_LastError = Enums::OperationError::WriteFileFailed;
            return false;
        }

        m_LastError = Enums::OperationError::Success;
        return true;
    }

    bool WindowsFile::Write(const void* input, usize size) noexcept
    {
        if (m_FileReadMode == Enums::ReadMode::Read)
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Trying to write into read-only file!");
            m_LastError = Enums::OperationError::FileIsReadOnly;
            return false;
        }

        DWORD bytesWritten;
        if (!WriteFile(m_File, input, static_cast<DWORD>(size), &bytesWritten, nullptr))
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Failed to write in file! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
            m_LastError = Enums::OperationError::WriteFileFailed;
            return false;
        }

        m_LastError = Enums::OperationError::Success;
        return true;
    }

    usize WindowsFile::Tell() const noexcept
    {
        LARGE_INTEGER li;
        li.QuadPart = 0;
        if (!SetFilePointerEx(m_File, li, &li, FILE_CURRENT))
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Failed to get file position! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
            return 0xFFFFFFFFFFFFFFFF;
        }
        return static_cast<usize>(li.QuadPart);
    }

    bool WindowsFile::Seek(usize position) noexcept
    {
        LARGE_INTEGER li;
        li.QuadPart = static_cast<LONGLONG>(position);
        if (!SetFilePointerEx(m_File, li, nullptr, FILE_BEGIN))
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Failed to seek file! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
            m_LastError = Enums::OperationError::SeekFailed;
            return false;
        }

        m_EndOfFile = position >= m_FileInfo.Size;
        m_LastError = Enums::OperationError::Success;
        return true;
    }

    bool WindowsFile::SeekToLine(usize line) noexcept
    {
        if (!Seek(0)) return false;

        String buf;
        u32 currentLine = 0;
        while (currentLine < line)
        {
            if (Read(buf, Enums::WordReadMode::Line, "")) continue;
            m_LastError = Enums::OperationError::EndOfFile;
            return false;
        }
        return true;
    }

    bool WindowsFile::Flush() noexcept
    {
        if (FlushFileBuffers(m_File)) return true;
        Logging::Logger::Error(Logging::Logger::Module::Core,
            "File flush failed! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
        m_LastError = Enums::OperationError::FlushFailed;
        return false;
    }

    void WindowsFile::OpenFile(const StringView& path) noexcept
    {
        DWORD readMode = FileReadModeToDWORD();
        if (readMode == 0)
        {
            m_LastError = Enums::OperationError::UnknownReadMode;
            return;
        }
        m_File = CreateFileA(AsChar7(path.String()), readMode,
            0, nullptr,
            m_FileReadMode == Enums::ReadMode::Read ? OPEN_EXISTING : OPEN_ALWAYS,
            m_FileReadMode == Enums::ReadMode::Read ? FILE_ATTRIBUTE_READONLY : FILE_ATTRIBUTE_NORMAL,
            nullptr);

        if (m_File == INVALID_HANDLE_VALUE)
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "File can not be opened! Win32 error: {:#18x}", static_cast<u32>(GetLastError()));
            return;
        }

        BY_HANDLE_FILE_INFORMATION info;
        if (!GetFileInformationByHandle(m_File, &info))
        {
            Logging::Logger::Error(Logging::Logger::Module::Core,
                "Could not gather file's [\"{}\"] information! Win32 error: {:#18x}",
                path, static_cast<u32>(GetLastError()));
            CloseHandle(m_File);
            m_File = INVALID_HANDLE_VALUE;
            return;
        }

        m_FileInfo.Name = path.ToString<String::AllocatorType>();
        PathStripPathA(reinterpret_cast<LPSTR>(m_FileInfo.Name.String()));
        m_FileInfo.Size = static_cast<usize>(info.nFileSizeLow) | (static_cast<usize>(info.nFileSizeHigh) << 32);
        m_FileInfo.CreationTime = WindowsFileTimeToTimepoint(info.ftCreationTime);
        m_FileInfo.LastWriteTime = WindowsFileTimeToTimepoint(info.ftCreationTime);

        m_LastError = Enums::OperationError::Success;
    }
}

MC::Memory::Scope<Details::FileBase> Details::FileBase::Create(const MC::StringView& path, Enums::ReadMode readMode) noexcept
{
    return Memory::MakeScope<Windows::Filesystem::WindowsFile>(path, readMode);
}
