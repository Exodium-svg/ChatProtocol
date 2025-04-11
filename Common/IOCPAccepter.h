#pragma once
#include "IOCPConnection.h"
#include <string>
#include <vector>
#include <atomic>
#include <mutex>
#include <functional>

struct AcceptContext {
	std::vector<std::string>* vBlockedIps;
	std::mutex* mIpLock;

public:
	AcceptContext(std::mutex* mIpLock, std::vector<std::string>* vBlockedIps): mIpLock(mIpLock), vBlockedIps(vBlockedIps){}
};

int CALLBACK AcceptCondition(
	LPWSABUF lpCallerId, LPWSABUF, LPQOS, LPQOS,
	LPWSABUF, LPWSABUF, GROUP FAR*, DWORD_PTR dwCallbackData);


class DLL_SPEC IOCPAccepter
{
private:
	const SOCKET m_hListener;
	char m_cAddress[INET_ADDRSTRLEN];
	const uint16_t m_nPort;
	const HANDLE m_hIOCP;
	void(*onConnect)(IOCPConnection* pConn);
	std::mutex m_mIpLock;
	std::vector<std::string> m_vIpBlocked; // need a mutex to protect it.
	std::atomic_bool m_bAlive;
	std::thread m_tMain;
public:
	IOCPAccepter(const HANDLE hIOCP, const char* pAddress, const uint16_t nPort, void(*onConnect)(IOCPConnection* pConn));
	~IOCPAccepter();
	void BlockAddress(const char* pAddress);

private:
	void AcceptLoop();
};
