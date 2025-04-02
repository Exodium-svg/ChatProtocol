#pragma once
#include "pch.h"
#include <fileapi.h>
#include "Stream.h";
class DLL_SPEC FileStream : public Stream
{
private:
	const HANDLE m_hFile;
	const bool m_bReadable;
	const bool m_bWriteable;
	size_t m_nCursor;
	size_t m_nFileSize;
public:
	FileStream(const HANDLE hFile, const bool bReadable, const bool bWriteable): 
		m_hFile(hFile), m_bReadable(bReadable), m_bWriteable(bWriteable), m_nCursor(0), m_nFileSize(GetFileSize(hFile, NULL)) {}
	~FileStream() { CloseHandle(m_hFile); }
	virtual size_t GetCursor() { return m_nCursor; }
	virtual void SetCursor(const size_t position);
	virtual void IncrementCursor(const size_t count);
	virtual size_t Write(const void* pBuff, const size_t length);
	virtual size_t Read(void* pBuff, const size_t length);
	virtual void Flush();
	virtual bool Readable() { return m_bReadable; };
	virtual bool WriteAble() { return m_bWriteable; };
};

