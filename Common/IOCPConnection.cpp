#include "pch.h"
#include "IOCPConnection.h"

void IOCPConnection::dispatch(const void* pData, DWORD len) noexcept
{
	// maybe use same block of memory?
	IOState* pIoState = new IOState();

	pIoState->pConn = this;
	pIoState->wsaBuff.buf = reinterpret_cast<char*>(malloc(len));
	memcpy_s(reinterpret_cast<void*>(pIoState->wsaBuff.buf), len, pData, len);

	pIoState->wsaBuff.len = len;
	pIoState->eType = EventType::Send;

	int nResult = WSASend(hSocket, &pIoState->wsaBuff, 1, nullptr, NULL, (OVERLAPPED*)pIoState, NULL);

	if (nResult != NO_ERROR)
		dispatchDisconnect();
}
