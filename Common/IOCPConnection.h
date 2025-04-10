#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <mutex>
#include "IOState.h"
#pragma comment(lib, "Ws2_32.lib")

// Peek header read
struct DLL_SPEC IOCPConnection
{
	Handle hHandle;
	const sockaddr_in addrIn;
	const SOCKET hSocket;
	const HANDLE hIOCP;
	BOOL bConnected; // has to be atomic

	IOCPConnection(SOCKET hSocket, const sockaddr_in inAddr) : hSocket(hSocket), addrIn(inAddr) {
		hHandle = NULL;
		bConnected = TRUE;
	}

	void dispatch(const void* pBuff, DWORD dwSize) noexcept;
	template<typename Obj>
	void dispatchMsg(const Obj& ref) noexcept {
		static_assert(std::is_trivially_copyable<Obj>(), "Cannot serialize non-trivial types");
		dispatch(&ref, sizeof(Obj));
	}
	
	void dispatchDisconnect() {
		IOState* pIoState = new IOState();
		pIoState->wsaBuff.buf = nullptr;

		pIoState->wsaBuff.len = NULL;
		pIoState->eType = EventType::Disconnect;
		pIoState->pConn = this;

		PostQueuedCompletionStatus(hIOCP, 0, (ULONG_PTR)this, (OVERLAPPED*)pIoState);
	}

	void Listen() {

	}

};

