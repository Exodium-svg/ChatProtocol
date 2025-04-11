#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <atomic>
#include "IOState.h"
#include "netheader.h"
#pragma comment(lib, "Ws2_32.lib")

// Peek header read
class DLL_SPEC IOCPConnection
{
private:
	void dispatch(const void* pBuff, DWORD dwSize) noexcept;
	const sockaddr_in addrIn;
	const SOCKET hSocket;
	const HANDLE hIOCP;
#ifdef _DEBUG
	bool m_bListening;
#endif // _DEBUG
public:
	Handle hHandle;
	std::atomic_bool bConnected;

	IOCPConnection(SOCKET hSocket, const sockaddr_in inAddr, const HANDLE hIOCP) 
		: hSocket(hSocket), addrIn(inAddr), hIOCP(hIOCP), bConnected(true), hHandle(NULL){
#ifdef _DEBUG
		m_bListening = false;
#endif
	}

	
	template<typename Obj>
	std::enable_if_t<std::is_trivially_copyable_v<Obj>, void>
	dispatchMsg(const Obj& ref) noexcept { dispatch(&ref, sizeof(Obj)); }
	
	void dispatchDisconnect();
	void close();
	void Listen();

};

