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
	pIoState->nMemLen = static_cast<unsigned long>(len);
	pIoState->eType = EventType::Send;

	int nResult = WSASend(hSocket, &pIoState->wsaBuff, 1, nullptr, NULL, (OVERLAPPED*)pIoState, NULL);

	if (nResult != NO_ERROR)
		dispatchDisconnect();
}

void IOCPConnection::dispatchDisconnect()
{
	IOState* pIoState = new IOState();
	pIoState->wsaBuff.buf = nullptr;
	pIoState->wsaBuff.len = NULL;
	pIoState->eType = EventType::Disconnect;
	pIoState->pConn = this;
	pIoState->nMemLen = NULL;
	PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)this, (OVERLAPPED*)pIoState);
}

void IOCPConnection::close() { closesocket(hSocket); }

void IOCPConnection::Listen()
{
#ifdef _DEBUG
	if (m_bListening == true)
		throw std::runtime_error("Connection is already in a listening state");
#endif // DEBUG

	IOState* pIoState = new IOState();

	pIoState->eType = EventType::Header;
	pIoState->wsaBuff.buf = reinterpret_cast<char*>(malloc(sizeof(NET_MESSAGE)));
	pIoState->wsaBuff.len = sizeof(NET_MESSAGE);
	pIoState->pConn = this;

	DWORD dwFlags = MSG_PEEK;

	int nResult = WSARecv(hSocket, &pIoState->wsaBuff, 1, nullptr, &dwFlags, pIoState, nullptr);

	if (nResult != NO_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		dispatchDisconnect();
}
