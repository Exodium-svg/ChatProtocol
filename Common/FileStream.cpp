#include "pch.h"
#include "FileStream.h"

void FileStream::SetCursor(const size_t position)
{
    DWORD nNewPos = SetFilePointer(m_hFile, position, NULL, FILE_BEGIN);

    if (nNewPos == INVALID_SET_FILE_POINTER)
        throw std::runtime_error("Invalid file position");

    m_nCursor = nNewPos;
}

void FileStream::IncrementCursor(const size_t count)
{
    DWORD nNewPos = SetFilePointer(m_hFile, count, NULL, FILE_CURRENT);

    if (nNewPos == INVALID_SET_FILE_POINTER)
        throw std::runtime_error("Invalid file position");

    m_nCursor = nNewPos;
}

size_t FileStream::Write(const void* pBuff, const size_t length)
{
    size_t nBytesWritten;
    BOOL result = WriteFile(m_hFile, pBuff, length, reinterpret_cast<LPDWORD>(&nBytesWritten), NULL);

    if (result == FALSE) {
        std::ostringstream sstream;
        sstream << "Failed to read file error: " << GetLastError();

        throw std::runtime_error(sstream.str());
    }

    m_nCursor += length;
    return nBytesWritten;
}

size_t FileStream::Read(void* pBuff, const size_t length)
{
    // We don't need to check, since if it is write only it will fail and throw an exception anyways.
    size_t nBytesRead;
    BOOL result = ReadFile(m_hFile, pBuff, length, reinterpret_cast<LPDWORD>(&nBytesRead), NULL);

    if (result == FALSE) {
        std::ostringstream sstream;
        sstream << "Failed to read file error: " << GetLastError();

        throw std::runtime_error(sstream.str());
    }

    m_nCursor += length;
    return nBytesRead;
}

void FileStream::Flush()
{
    BOOL result = FlushFileBuffers(m_hFile);

    if (!result) {
        std::ostringstream sstream;
        sstream << "Failed to flush buffer error: " << GetLastError();

        throw std::runtime_error(sstream.str());
    }
        
}
