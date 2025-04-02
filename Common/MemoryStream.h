#pragma once
#include "Stream.h"
#include <vector>
class DLL_SPEC MemoryStream : public Stream
{
protected:
	std::vector<char> m_vBuff;
	size_t m_nPosition;
public:
	MemoryStream(std::vector<char> vBuff);
	MemoryStream(const char* pBuff, const size_t nSize);

	void Grow(const size_t length) { m_vBuff.resize(m_vBuff.size() + length); }
	virtual size_t GetCursor() { return m_nPosition; }
	virtual void IncrementCursor(const size_t count) { m_nPosition += count; }
	virtual void SetCursor(const size_t position) { m_nPosition = position; }
	virtual size_t Write(const void* pBuff, const size_t length);
	virtual size_t Read(void* pBuff, const size_t length);
	virtual void Flush() { return; }
	virtual bool Readable() { return true; }
	virtual bool WriteAble() { return true; }
};

