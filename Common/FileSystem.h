#pragma once
#include "pch.h"
#include "FilePath.h"
#include "FileStream.h"
class DLL_SPEC FileSystem
{
public:
	static bool Create(const FilePath& path);
	static bool Exists(const FilePath& path);
	static bool Delete(const FilePath& path);
	static FileStream Read(const FilePath& path);
	static FileStream Write(const FilePath& path);
};

