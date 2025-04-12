#pragma once
#include <winsock2.h>
#include <windows.h>
#include <mswsock.h>
#include <processthreadsapi.h> 
#include "Socket.h"
#include <sstream>
#include <string>
#include <stdint.h>
#include "IOCPConnection.h"
//#pragma comment(lib, "ws2_32.lib")

typedef HANDLE IOCP_t;

struct DLL_SPEC IOCPState {
	IOCP_t hIOCP;
	std::atomic<uint32_t> nThreads;
	std::atomic_bool bAlive;
	std::atomic<uint64_t> nBytesSent;
	std::atomic<uint64_t> nBytesReceived;
	void(*onReceive)(IOCPConnection* pConn, const NET_MESSAGE* pMsg);
	void(*onDisconnect)(IOCPConnection* pConn);
};
namespace IOCP {
	DWORD DLL_SPEC WINAPI IOCPWorkerThread(LPVOID lpParam);
	DLL_SPEC IOCPState* InitializeIOCP(uint32_t nThreads);
	DLL_SPEC void ShutdownIOCP(IOCPState* pIOCP);
};
