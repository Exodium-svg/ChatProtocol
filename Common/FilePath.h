#pragma once
#include "pch.h"
#include <string>
#include <fileapi.h>
struct DLL_SPEC FilePath
{
private:
	std::string m_sFilePath;
public:
	FilePath(const std::string& sFilePath);
	const std::string GetExtension() const;
	const char* GetFilePath() const;
	void SetFilePath(const std::string& filePath);
	void SetExtension(const std::string& extension);
};

