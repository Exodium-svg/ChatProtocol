#pragma once
#include <thread>
#include <stdatomic.h>
#include "pch.h"
#include "Socket.h"
class DLL_SPEC SocketAccepter
{
private:
	const SOCKET m_hListener;
	const std::function<void(Socket*)> m_onConnect;
	std::thread m_listenThread;
	uint64_t m_handleCount = 0;
	std::atomic_bool m_bIsAlive;
public:
	SocketAccepter(const char* pAddress, const char* nPort, const std::function<void(Socket*)> onConnect);
private:
	void ListenLoop();
};

