#pragma once
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <stdint.h>
#include <string>
#include <sstream>
#include <functional>
#include <windows.h>
#include <exception>
#include <atomic>
#include <mutex>
#include "netheader.h"
#include "pch.h"
#pragma comment(lib, "Ws2_32.lib")

static WSADATA* wsadata = new WSAData();


void DLL_SPEC InitializeNetwork();
void DLL_SPEC DeInitializeNetwork();
BOOL DLL_SPEC NetworkReady();
void CALLBACK onCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

// TODO's:
// Rework handles to be a pointer instead...
struct DLL_SPEC Socket
{
	std::mutex m_mtWsaEvent;
	Handle m_handle;
	SOCKET m_hSocket; // socket handle;
	const uint16_t m_nPort;
	char m_cAddress[14];
	char m_cBuff[sizeof(NET_MESSAGE)];
	void(*m_onReceive)(Socket* pConn, const NET_MESSAGE, const void*) = nullptr;
	std::atomic_bool bConnected;
private:
	
	OVERLAPPED wsaEvent{};
	WSAPOLLFD wsaPollfd{};
	WSABUF wsaRoutine{};
public:
	Socket(SOCKET hSocket, const uint16_t nPort, const char* pAddress);
	Socket(const char* pAddress, const uint16_t nPort);
	void Reconnect();
	void pollEvents();
	~Socket();
	BOOL connected() { return bConnected.load(); }; // has to be atomic?
	DWORD send(const void* pData, DWORD size);
	template <typename T>
	DWORD send(const T& obj) {
		static_assert(std::is_trivially_copyable_v<T>, "Cannot serialize non-trivial types");
		return send(&obj, sizeof(T));
	}
	const DWORD receive(void* pData, DWORD size);
	void onReceive(void(*onReceive)(Socket*, const NET_MESSAGE, const void*));
	void setCompletionRoutine(void (*completionRoutine)(DWORD, DWORD, LPWSAOVERLAPPED, DWORD));
	void disconnect();
};

