#include "pch.h"
#include "FilePath.h"

FilePath::FilePath(const std::string& sFilePath): m_sFilePath(sFilePath) {}

const std::string FilePath::GetExtension() const
{
    size_t nExtStart = m_sFilePath.find_last_of('.', 0);

    if (nExtStart == std::string::npos)
        return "";

    return m_sFilePath.substr(nExtStart, m_sFilePath.length());
}

const char* FilePath::GetFilePath() const { return m_sFilePath.c_str(); }

void FilePath::SetFilePath(const std::string& filePath) { m_sFilePath = filePath; }

void FilePath::SetExtension(const std::string& extension)
{
    size_t nExtStart = m_sFilePath.find_last_of(".");

    if (nExtStart == std::string::npos)
        m_sFilePath += '.' + extension;
    else
        m_sFilePath.substr(0, nExtStart) + '.' + extension;
}
