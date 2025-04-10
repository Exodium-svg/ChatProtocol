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
void CALLBACK onReceiveHeaderRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);
void CALLBACK onReceiveMessageRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags);

enum DLL_SPEC SockState {
	ReceiveHeader,
	ReceiveMessage,
	Disconnecting,
	Disconnected,
};

struct DLL_SPEC Socket {
	struct DLL_SPEC IOState {
		SockState sockState;
		WSABUF wsaBuff;
		uint64_t nMemSize;
		Socket* pSocket;
	};

	Handle hHandle;
	HANDLE hMutex;
	SOCKET hSocket;
	sockaddr_in inAddress;
	IOState ioState;
	LPOVERLAPPED pOverlapped;
	BOOL bConnected;
	BOOL bIOCPSocket;
	void(*onReceive)(Socket* pSocket, const NET_MESSAGE* pMsg);

	Socket(sockaddr_in address, SOCKET hSocket);
	Socket(const char* pAddress, const uint16_t nPort);
	~Socket();
	std::unique_ptr<char> GetAddress();
	uint16_t GetPort();
	DWORD send(const void* pBuff, DWORD dwSize);
	template<typename Obj>
	DWORD send(const Obj& ref) {
		static_assert(std::is_trivially_copyable_v<Obj>, "Cannot serialize non-trivial types");
		return send(&ref, sizeof(Obj));
	}
	const DWORD receive(void* pBuff, DWORD dwSize);
	void setOnReceive(void(*onReceive)(Socket* pSocket, const NET_MESSAGE* pMsg));
	void startCompletionRoutine(SockState eState = SockState::ReceiveHeader);
	void bindToIOCP(HANDLE hIOCP);
	void disconnect();
};

void CloseSocket(Socket* pSocket);