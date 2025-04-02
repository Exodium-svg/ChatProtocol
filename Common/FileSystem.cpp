#include "pch.h"
#include "FileSystem.h"

bool FileSystem::Create(const FilePath& path)
{
    return CloseHandle
    (CreateFileA
        (
        path.GetFilePath(),
        NULL,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
        )
    );
}

bool FileSystem::Exists(const FilePath& path)
{
    DWORD nFileAttrs = GetFileAttributesA(path.GetFilePath());

    if (nFileAttrs == INVALID_FILE_ATTRIBUTES)
        return false;

    return true;
}

bool FileSystem::Delete(const FilePath& path) { return DeleteFileA(path.GetFilePath()); }

FileStream FileSystem::Read(const FilePath& path)
{
    HANDLE hFile = CreateFileA(path.GetFilePath(),
        GENERIC_READ,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    return std::move(FileStream(hFile, true, false));
}

FileStream FileSystem::Write(const FilePath& path)
{
    HANDLE hFile = CreateFileA(path.GetFilePath(),
        GENERIC_WRITE,
        0,
        NULL,
        OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    return std::move(FileStream(hFile, false, true));
}
