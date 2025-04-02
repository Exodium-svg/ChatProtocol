#include "pch.h"
#include "Socket.h"

//static WSADATA* wsadata;

void InitializeNetwork()
{
	if (NetworkReady())
		throw std::runtime_error("Network is already intialized");

	int result = WSAStartup(MAKEWORD(2, 2), wsadata);

	if (result != 0) {
		std::ostringstream sstream;

		sstream << "Failed to intialize Winsock2 error: ";
		sstream << result;

		throw std::runtime_error(sstream.str().c_str());
	}
}

void DeInitializeNetwork() { WSACleanup(); }

BOOL NetworkReady()
{
	//if (wsadata != nullptr)
	//	return TRUE;
	return FALSE;
}

void CALLBACK onCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	if (lpOverlapped->Pointer == nullptr)
		throw "";

	Socket* pSocket = reinterpret_cast<Socket*>(lpOverlapped->Pointer);

	// If an error accurs that means that we are probably not connected anymore... Cleanup.
	switch (dwError) {
		case ERROR_SUCCESS:
		break;
		case WSAECONNRESET:
			pSocket->disconnect();
			return;
		case WSAETIMEDOUT: // Timed out
			pSocket->disconnect();
			return;
		case WSAENETDOWN: // Forcibly closed
			pSocket->disconnect();
			return;
		case WSAECONNABORTED: // Connection aborted
			pSocket->disconnect();
			return;
		default:
			throw "Unknown system error(?)";
	}

	if (pSocket == nullptr || !pSocket->connected()) {
		throw "Invalid socket state whilst trying to receive data(?)";
	}
	// We are out of sync.
	if (cbTransferred != sizeof(NET_MESSAGE)) {
		pSocket->disconnect();
		return;
	}

	const NET_MESSAGE* pHeader = reinterpret_cast<const NET_MESSAGE*>(pSocket->m_cBuff);

	// If the checksum is wrong, something is invalid. Are we out of sync?
	if (pHeader->checksum != pHeader->calculateChecksum()) {
		pSocket->disconnect();
		return;
	}

	void* pBuff = malloc(pHeader->length);

	// We check if it has all been sent in one packet, or we just pretend it hasn't been sent at all.
	const DWORD nReceivedBytes = pSocket->receive(pBuff, pHeader->length);

	if (nReceivedBytes != pHeader->length) {
		pSocket->disconnect();
		return;
	}

	try {
		if(pSocket->m_onReceive != nullptr)
			pSocket->m_onReceive(pSocket, *pHeader, pBuff);
	}
	catch (const std::exception& exception) {
		// handling errors here?
	}

	free(pBuff);

	if (pSocket->connected())
		pSocket->setCompletionRoutine(onCompletionRoutine);
}

void Socket::pollEvents()
{
	int result = WSAPoll(&wsaPollfd, 1, 999);

	if (result == SOCKET_ERROR)
		bConnected.store(false);
	else
		bConnected.store(false);

	if (wsaPollfd.revents & POLLERR)
		bConnected.store(false);

	if (wsaPollfd.revents & POLLHUP)
		bConnected.store(false);
}

Socket::Socket(const SOCKET hSocket, const uint16_t nPort, const char* pAddress): m_handle(NULL), m_hSocket(hSocket), m_nPort(nPort)
{
	ZeroMemory(&wsaEvent, sizeof(wsaEvent));
	constexpr int nAddrLen = sizeof(m_cAddress) - 1;
	memcpy_s(static_cast<char*>(m_cAddress), nAddrLen, pAddress, strlen(pAddress));
	m_cAddress[sizeof(m_cAddress) -1] = '\0';

	constexpr size_t headerLen = sizeof(NET_MESSAGE);

	wsaEvent.Pointer = (PVOID)this;
	
	pollEvents();
}

Socket::Socket(const char* pAddress, const uint16_t nPort): m_handle(0), m_hSocket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)), m_nPort(nPort)
{
	//constexpr size_t headerLen = sizeof(NET_MESSAGE);
	ZeroMemory(&wsaEvent, sizeof(wsaEvent));
	ZeroMemory(&m_cBuff, sizeof(m_cBuff));

	wsaEvent.Pointer = (PVOID)this;
	constexpr int nAddrLen = sizeof(m_cAddress);
	if (strlen(pAddress) < nAddrLen) {
		memcpy_s(static_cast<char*>(m_cAddress), nAddrLen, pAddress, strlen(pAddress));
		m_cAddress[strlen(pAddress)] = '\0';  // Null-terminate the string
	}
	else {
		throw std::runtime_error("Address too large for m_cAddress");
	}

	if (m_hSocket == INVALID_SOCKET) {
		throw std::runtime_error("Failed to create socket");
	}

	sockaddr_in addrTarget{};
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(nPort); // ensure correct endian-ness

	if (inet_pton(AF_INET, pAddress, &addrTarget.sin_addr) != 1) {
		throw std::runtime_error("Invalid address");
	}

	int nResult = connect(m_hSocket, reinterpret_cast<sockaddr*>(&addrTarget), sizeof(sockaddr_in));

	if (nResult == SOCKET_ERROR) {
		std::ostringstream sstream;
		sstream << "Failed to connect to endpoint: " << pAddress << ":" << nPort << " error: " << WSAGetLastError();

		throw std::runtime_error(sstream.str());
	}
	
	bConnected.store(true);
}

Socket::~Socket()
{
	if (connected())
		disconnect();
	else
		closesocket(m_handle);
}

DWORD Socket::send(const void* pData, DWORD size)
{
	if (connected() == FALSE)
		return 0;

	WSABUF wsaBuff{};
	wsaBuff.buf = reinterpret_cast<char*>(const_cast<void*>(pData));
	wsaBuff.len = size;

	DWORD dwReceivedBytes;
	int nResult = WSASend(this->m_hSocket, &wsaBuff, 1, &dwReceivedBytes, MSG_OOB, &wsaEvent, NULL);
	
	if (nResult == SOCKET_ERROR) {
		std::ostringstream sstream;
		sstream << "Failed to send data error: " << WSAGetLastError();

		throw std::runtime_error(sstream.str());
	}

	if (nResult != 0 || size != dwReceivedBytes)
		bConnected.store(false);

	return dwReceivedBytes;
}

const DWORD Socket::receive(void* pData, DWORD size)
{
	if (connected() == FALSE)
		return 0;

	DWORD receivedBytes = recv(m_hSocket, reinterpret_cast<char*>(pData), size, MSG_OOB);

	if (receivedBytes < 0)
		bConnected.store(false);

	return receivedBytes;
}

void Socket::onReceive(void(*onReceive)(Socket*, const NET_MESSAGE, const void*))
{
	this->m_onReceive = onReceive;
	this->setCompletionRoutine(onCompletionRoutine);
}

void Socket::setCompletionRoutine(void(*completionRoutine)(DWORD, DWORD, LPWSAOVERLAPPED, DWORD))
{
	DWORD dwHeaderSize = (DWORD)sizeof(NET_MESSAGE);
	DWORD dwFlags = MSG_PEEK;
	// should have it's own wsaBUFF for routines.

	// Array automatically decays into a pointer.
	wsaRoutine.buf = m_cBuff;
	wsaRoutine.len = sizeof(m_cBuff);

	//ZeroMemory(&wsaEvent, sizeof(wsaEvent));

	// We are peeking so we can read everything upon receiving it.
	int nResult = WSARecv(m_hSocket, &wsaRoutine, 1, &dwHeaderSize, &dwFlags, &wsaEvent, completionRoutine);

	// If a operation is handled in Async, then the result code would be that it is pending.
	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		bConnected.store(false);

}

void Socket::disconnect()
{
	// we don't really care about the result here
	if(connected())
		shutdown(m_hSocket, SD_BOTH);

	bConnected.store(false);
	closesocket(m_hSocket);
}

