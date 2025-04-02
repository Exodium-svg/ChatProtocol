#pragma once
#include "pch.h"
#include <stdexcept>
#include <sstream>
class DLL_SPEC Stream
{
public:
	template<typename T>
	Stream& operator << (const T& obj) {
		static_assert(std::is_trivially_copyable_v<T>, "Cannot serialize non-trivial types");

		size_t nBytesWritten = Write(reinterpret_cast<const void*>(&obj), sizeof(T));

		if (nBytesWritten != sizeof(T)) {
			std::ostringstream sstream;

			sstream << "Failed to write all bytes, only wrote " << nBytesWritten << "out of " << sizeof(T) << " bytes";
			throw std::runtime_error(sstream.str());
		}
	}

	template<typename T>
	Stream& operator >> (T& obj) {
		static_assert(std::is_trivially_copyable_v<T>, "Cannot serialize non-trivial types");

		size_t nBytesRead = Read(reinterpret_cast<void*>(&obj), sizeof(T));

		if (nBytesRead != sizeof(T)) {
			std::ostringstream sstream;

			sstream << "Failed to read all bytes, only wrote " << nBytesRead << "out of " << sizeof(T) << " bytes";
			throw std::runtime_error(sstream.str());
		}
	}

	virtual size_t GetCursor() = 0;
	virtual void SetCursor(const size_t position) = 0;
	virtual void IncrementCursor(const size_t count) = 0;
	virtual size_t Write(const void* pBuff, const size_t length) = 0;
	virtual size_t Read(void* pBuff, const size_t length) = 0;
	virtual void Flush() = 0;
	virtual bool Readable() = 0;
	virtual bool WriteAble() = 0;
};

