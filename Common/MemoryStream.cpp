#include "pch.h"
#include "MemoryStream.h"

MemoryStream::MemoryStream(std::vector<char> vBuff)
{
	// uses copy constructor.
	m_vBuff = vBuff;
	m_nPosition = 0;
}

MemoryStream::MemoryStream(const char* pBuff, const size_t nSize)
{
	m_vBuff = std::vector<char>(nSize);

	memcpy_s(m_vBuff.data(), nSize, pBuff, nSize);
	m_nPosition = 0;
}

size_t MemoryStream::Write(const void* pBuff, const size_t length)
{
	errno_t err = memcpy_s(m_vBuff.data() + m_nPosition, length, pBuff, m_vBuff.size());

	if (err != 0)
		throw std::runtime_error("Unable to write beyond stream.");

	m_nPosition += length;
}

size_t MemoryStream::Read(void* pBuff, const size_t length)
{
	errno_t err = memcpy_s(pBuff, length, m_vBuff.data(), m_vBuff.size());

	if(err != 0)
		throw std::runtime_error("Unable to read beyond stream.");

	m_nPosition += length;
}
