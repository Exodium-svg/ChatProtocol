#pragma once
#include <stdexcept>
#include "Stream.h"
#include "Socket.h"
class DLL_SPEC NetworkStream : public Stream
{
private:
	Socket* m_socket;
public:
	NetworkStream(Socket* socket) : m_socket(socket) {};
	virtual size_t GetCursor() { throw std::exception("Not implemented"); };
	virtual void SetCursor(const size_t position) { throw std::exception("Not implemented"); };
	virtual void IncrementCursor(const size_t count) { throw std::exception("Not implemented"); };
	virtual size_t Write(const void* pBuff, const size_t length);
	virtual size_t Read(void* pBuff, const size_t length);
	virtual void Flush() { throw std::exception("Not implemented"); };
	virtual bool Readable() { return true; };
	virtual bool WriteAble() { return true; };
};

