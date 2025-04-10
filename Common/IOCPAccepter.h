#pragma once
#include "IOCPConnection.h"
#include <string>
#include <vector>


int CALLBACK AcceptCondition(
	LPWSABUF lpCallerId, LPWSABUF, LPQOS, LPQOS,
	LPWSABUF, LPWSABUF, GROUP FAR*, DWORD_PTR dwCallbackData)
{
	sockaddr_in* remoteAddr = reinterpret_cast<sockaddr_in*>(lpCallerId->buf);
	std::vector<std::string>* vIpBlocked = reinterpret_cast<std::vector<std::string>*>(dwCallbackData);

	char cAddress[INET_ADDRSTRLEN];

	//TODO we do not support ipv6 here.....
	inet_ntop(AF_INET, &remoteAddr->sin_addr, cAddress, sizeof(cAddress));


	for (size_t i = 0; i < vIpBlocked->size(); i++) {
		const std::string& sAddress = vIpBlocked->at(i);

		if (sAddress == cAddress)
			return CF_REJECT;
	}

	return CF_ACCEPT;
}


class IOCPAccepter
{
private:
	const SOCKET m_hSocket;
	char m_cAddress[INET_ADDRSTRLEN];
	const uint16_t m_nPort;

	const void(*onConnect)(IOCPConnection* pConn);
	std::vector<std::string> m_vIpBlocked; // need a mutex to protect it.
	std::thread m_tMain;
public:
	IOCPAccepter(const char* pAddress, const uint16_t nPort, const void(*onConnect)(IOCPConnection* pConn))
		: m_nPort(nPort), onConnect(onConnect), m_hSocket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)), m_vIpBlocked(std::vector<std::string>()) {
		size_t nStrlen = strlen(pAddress);

		if (sizeof(m_cAddress) < nStrlen)
			throw std::runtime_error("Invalid ip string");

		memcpy_s(m_cAddress, sizeof(m_cAddress), pAddress, nStrlen);

		// connect socket.
		
	}

	void BlockAddress(const char* pAddress) {
		m_vIpBlocked.emplace_back(pAddress);
	}

private:
	void AcceptLoop() {
		int addrLen = sizeof(sockaddr_in);
		while (true) {
			sockaddr_in addrIn{};
			SOCKET hRemoteSock = WSAAccept(m_hSocket, reinterpret_cast<sockaddr*>(&addrIn), &addrLen, AcceptCondition, (DWORD_PTR) & m_vIpBlocked);

			if (hRemoteSock == INVALID_SOCKET)
				continue;

			IOCPConnection* pConn = new IOCPConnection(hRemoteSock, addrIn);

			onConnect(pConn);
		}
	}
};
