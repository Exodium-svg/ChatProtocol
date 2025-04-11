#include "pch.h"
#include "IOCPAccepter.h"

IOCPAccepter::IOCPAccepter(const HANDLE hIOCP, const char* pAddress, const uint16_t nPort, void(*onConnect)(IOCPConnection* pConn))
	:m_hIOCP(hIOCP), m_nPort(nPort), onConnect(onConnect), m_hListener(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)), m_vIpBlocked(std::vector<std::string>()), m_bAlive(true) {
	size_t nStrlen = strlen(pAddress);

	if (sizeof(m_cAddress) <= nStrlen)
		throw std::runtime_error("Invalid ip string");

	memcpy_s(m_cAddress, sizeof(m_cAddress), pAddress, nStrlen);

	addrinfo* result = nullptr;
	addrinfo hints{};

	hints.ai_family = AF_INET;       // Use IPv4
	hints.ai_socktype = SOCK_STREAM; // Use TCP
	hints.ai_protocol = IPPROTO_TCP; // Explicitly set TCP protocol
	hints.ai_flags = AI_PASSIVE;     // Bind to wildcard address if `pAddress` is NULL

	std::string sPort = std::to_string(nPort);
	int nResult = getaddrinfo(pAddress, sPort.c_str(), &hints, &result);

	if (nResult != 0) {
		std::ostringstream sstream{};

		sstream << "Resolving host name failed, error: " << nResult;
		throw std::runtime_error(sstream.str());
	}

	if (m_hListener == INVALID_SOCKET)
		throw std::runtime_error("Failed to create socket");

	nResult = bind(m_hListener, result->ai_addr, result->ai_addrlen);
	freeaddrinfo(result);
	if (nResult != NO_ERROR)
		throw std::runtime_error("Failed to bind port, is port already in use?");

	nResult = listen(m_hListener, SOMAXCONN);

	if (nResult != NO_ERROR)
		throw std::runtime_error("Failed to listen to socket");

	m_tMain = std::thread(std::bind(&IOCPAccepter::AcceptLoop, this));
}

IOCPAccepter::~IOCPAccepter()
{
	m_bAlive = false;
	// when we close this it also signals the WSA function.
	closesocket(m_hListener);

	if (m_tMain.joinable()) {
#ifdef  _DEBUG
		throw std::runtime_error("Unable to join thread");
#endif //  _DEBUG

		m_tMain.join();
	}
}

void IOCPAccepter::BlockAddress(const char* pAddress)
{
	std::unique_lock <std::mutex> lock(m_mIpLock);
	m_vIpBlocked.emplace_back(pAddress);
}

void IOCPAccepter::AcceptLoop()
{
	int addrLen = sizeof(sockaddr_in);
	AcceptContext acceptCtx = AcceptContext(&m_mIpLock, &m_vIpBlocked);

	while (m_bAlive) {
		sockaddr_in addrIn{};
		SOCKET hRemoteSock = WSAAccept(m_hListener, reinterpret_cast<sockaddr*>(&addrIn), &addrLen, AcceptCondition, (DWORD_PTR)&acceptCtx);

		if (hRemoteSock == INVALID_SOCKET)
			continue;

		IOCPConnection* pConn = new IOCPConnection(hRemoteSock, addrIn, m_hIOCP);

		onConnect(pConn);
	}
}

int CALLBACK AcceptCondition(LPWSABUF lpCallerId, LPWSABUF, LPQOS, LPQOS, LPWSABUF, LPWSABUF, GROUP*, DWORD_PTR dwCallbackData)

{
	sockaddr_in* remoteAddr = reinterpret_cast<sockaddr_in*>(lpCallerId->buf);

	AcceptContext* ctx = reinterpret_cast<AcceptContext*>(dwCallbackData);

	std::vector<std::string>* vIpBlocked = ctx->vBlockedIps;

	char cAddress[INET_ADDRSTRLEN];

	//TODO we do not support ipv6 here fix(?)
	inet_ntop(AF_INET, &remoteAddr->sin_addr, cAddress, sizeof(cAddress));

	std::unique_lock<std::mutex> lock(*ctx->mIpLock);

	for (size_t i = 0; i < vIpBlocked->size(); i++) {
		const std::string& sAddress = vIpBlocked->at(i);

		if (sAddress == cAddress)
			return CF_REJECT;
	}

	return CF_ACCEPT;
}
