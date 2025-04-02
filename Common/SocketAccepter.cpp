#include "pch.h"
#include "SocketAccepter.h"

SocketAccepter::SocketAccepter(const char* pAddress, const char* pPort, std::function<void(Socket*)> onConnect):
	m_hListener(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)), m_onConnect(onConnect), m_bIsAlive(false)
{
	if (!NetworkReady())
		InitializeNetwork();

	addrinfo* result = nullptr;
	addrinfo hints{};

	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // Use TCP
	hints.ai_protocol = IPPROTO_TCP; // Explicitly set TCP protocol
	hints.ai_flags = AI_PASSIVE;     // Bind to wildcard address if `pAddress` is NULL

	int nResult = getaddrinfo(NULL, pPort, &hints, &result);

	if (nResult != 0) {
		std::ostringstream sstream{};

		sstream << "Resolving host name failed, error: " << nResult;
		throw std::runtime_error(sstream.str());
	}

	if (m_hListener == INVALID_SOCKET)
		throw std::runtime_error("Failed to create socket");

	nResult = bind(m_hListener, result->ai_addr, result->ai_addrlen);

	if (nResult != 0)
		throw std::runtime_error("Failed to bind port, is port already in use?");
	
	nResult = listen(m_hListener, SOMAXCONN);

	if (nResult != 0)
		throw std::runtime_error("Failed to listen to socket");

	m_listenThread = std::thread(std::bind(&SocketAccepter::ListenLoop, this));
}

void SocketAccepter::ListenLoop() {
	m_bIsAlive = true;
	sockaddr_in addrIn{};
	int nAddrSize = sizeof(sockaddr_in);

	while(m_bIsAlive) {
		SOCKET hRemoteSock = accept(m_hListener, reinterpret_cast<sockaddr*>(&addrIn), &nAddrSize);

		if (hRemoteSock == INVALID_SOCKET)
			continue;

		constexpr int addrLen = sizeof(addrIn.sin_zero);
		char cAddressBuf[addrLen];

		memcpy_s(cAddressBuf, addrLen, addrIn.sin_zero, addrLen);

		//TODO: dirty solution, we need a queue for already return handles, this way we'd never run out of handles.
		m_onConnect(new Socket(hRemoteSock, addrIn.sin_port, reinterpret_cast<char*>(&cAddressBuf)));
	}
}