#include "pch.h"
#include "NetworkStream.h"

size_t NetworkStream::Write(const void* pBuff, const size_t length)
{
	return static_cast<size_t>(m_socket->send(const_cast<void*>(pBuff), length));
}

size_t NetworkStream::Read(void* pBuff, const size_t length)
{
	return static_cast<size_t>(m_socket->receive(pBuff, length));
}
