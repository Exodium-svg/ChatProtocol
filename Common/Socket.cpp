#include "pch.h"
#include "Socket.h"

//static WSADATA* wsadata;

inline void ConnectToServer(Socket* pSocket) {
	sockaddr_in addrTarget{};
	addrTarget.sin_family = AF_INET;
	addrTarget.sin_port = htons(pSocket->m_nPort); // ensure correct endian-ness

	if (inet_pton(AF_INET, pSocket->m_cAddress, &addrTarget.sin_addr) != 1) {
		throw std::runtime_error("Invalid address");
	}

	int nResult = connect(pSocket->m_hSocket, reinterpret_cast<sockaddr*>(&addrTarget), sizeof(sockaddr_in));

	if (nResult == SOCKET_ERROR) {
		std::ostringstream sstream;
		sstream << "Failed to connect to endpoint: " << pSocket->m_cAddress << ":" << pSocket->m_nPort << " error: " << WSAGetLastError();

		throw std::runtime_error(sstream.str());
	}
}

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
	//TODO: implement me.
	//if (wsadata != nullptr)
	//	return TRUE;
	return FALSE;
}

inline void CloseSocket(Socket* pSocket) {
	if (pSocket->bConnected == false)
		return;

	pSocket->bConnected.store(false);
	closesocket(pSocket->m_hSocket);
	pSocket->m_hSocket = NULL;
}

struct PhaseTwo {
	Socket* pSocket;
	NET_MESSAGE msg;
};
void CALLBACK onPhaseTwoRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags) {
	if (lpOverlapped->Pointer == nullptr)
		throw "Completion routine should always be called with the Overlapped pointer set.";

	PhaseTwo* pPhase = reinterpret_cast<PhaseTwo*>(lpOverlapped);

	Socket* pSocket = pPhase->pSocket;
	NET_MESSAGE netMsg = pPhase->msg;

	lpOverlapped->Pointer = pSocket;

	free(pPhase);





	pSocket->setCompletionRoutine(onCompletionRoutine);
}
void CALLBACK onCompletionRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	if (lpOverlapped->Pointer == nullptr)
		throw "Completion routine should always be called with the Overlapped pointer set.";

	Socket* pSocket = reinterpret_cast<Socket*>(lpOverlapped->Pointer);
	std::lock_guard<std::mutex> lock(pSocket->m_mtWsaEvent);
	// safe guard
	if (!pSocket->connected()) {
		return;
	}

	switch (dwError) {
		case ERROR_SUCCESS:
			break;
		case WSAECONNRESET:
			CloseSocket(pSocket);
			return;
		case WSAETIMEDOUT: // Timed out
			CloseSocket(pSocket);
			return;
		case WSAENETDOWN: // Forcibly closed
			CloseSocket(pSocket);
			return;
		case WSAECONNABORTED: // Connection aborted
			CloseSocket(pSocket);
			return;
		default:
			throw "Unknown system error(?)";
	}

	// We are out of sync.
	// Best to just not question it, However when a shutdown message is received it will be caught in this condition as well.
	if (cbTransferred != sizeof(NET_MESSAGE)) {
		CloseSocket(pSocket);
		return;
	}

	const NET_MESSAGE* pHeader = reinterpret_cast<const NET_MESSAGE*>(pSocket->m_cBuff);

	// If the checksum is wrong, something is invalid. Are we out of sync?
	//if (pHeader->checksum != pHeader->calculateChecksum()) {
	//	pSocket->disconnect();
	//	shutdown(pSocket->m_hSocket, SD_BOTH);
	//	CloseSocket(pSocket);
	//	return;
	//}
	PhaseTwo* phase = (PhaseTwo*)malloc(sizeof(PhaseTwo));
	memcpy_s(&phase->msg, sizeof(NET_MESSAGE), pHeader, sizeof(NET_MESSAGE));

	pSocket->wsaEvent.Pointer = phase;

	//TODO have to move memory management outside of the method.
	pSocket->setCompletionRoutine(onPhaseTwoRoutine);

	//// Should be implemented in phase 2
	//bool bStackAlloc = false;
	//if (1024 > pHeader->length)
	//	bStackAlloc = true;

	//void* pBuff;
	//
	//if (bStackAlloc)
	//	pBuff = _alloca(pHeader->length);
	//else
	//	pBuff = malloc(pHeader->length);
	//
	//// We check if it has all been sent in one packet, or we just pretend it hasn't been sent at all.
	//const DWORD nReceivedBytes = pSocket->receive(pBuff, pHeader->length);

	//if (nReceivedBytes != pHeader->length) {
	//	if(!bStackAlloc)
	//		free(pBuff);

	//	pSocket->disconnect();
	//	return;
	//}

	//try {
	//	if(pSocket->m_onReceive != nullptr)
	//		pSocket->m_onReceive(pSocket, *pHeader, pBuff);
	//}
	//catch (const std::exception& exception) {
	//	// handling errors here?
	//}

	//if (!bStackAlloc)
	//	free(pBuff);

	//if (pSocket->connected())
	//	pSocket->setCompletionRoutine(onCompletionRoutine);
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
	bConnected.store(true);
	ZeroMemory(&wsaEvent, sizeof(wsaEvent));
	ZeroMemory(m_cBuff, sizeof(m_cBuff));
	constexpr int nAddrLen = sizeof(m_cAddress) - 1;
	memcpy_s(static_cast<char*>(m_cAddress), nAddrLen, pAddress, strlen(pAddress));
	m_cAddress[sizeof(m_cAddress) -1] = '\0';

	constexpr size_t headerLen = sizeof(NET_MESSAGE);

	wsaEvent.Pointer = (PVOID)this;
	
	//pollEvents();
}

Socket::Socket(const char* pAddress, const uint16_t nPort): m_handle(0), m_hSocket(socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)), m_nPort(nPort)
{
	ZeroMemory(&wsaEvent, sizeof(wsaEvent));
	ZeroMemory(m_cBuff, sizeof(m_cBuff));
	ZeroMemory(m_cAddress, sizeof(m_cAddress));

	bConnected.store(true);

	wsaEvent.Pointer = (PVOID)this;
	constexpr int nAddrLen = sizeof(m_cAddress);
	if (strlen(pAddress) < nAddrLen) {
		memcpy_s(static_cast<char*>(m_cAddress), nAddrLen, pAddress, strlen(pAddress));
		m_cAddress[strlen(pAddress)] = '\0';  // Null-terminate the string
	}
	else {
		bConnected.store(false);
	}

	if (m_hSocket == INVALID_SOCKET) {
		bConnected.store(false);
	}

	try {
		ConnectToServer(this);
		bConnected.store(true);
	}
	catch (const std::exception& ex) {
		bConnected.store(false);
	}
	
}

void Socket::Reconnect() { 
	try {
		ConnectToServer(this);
		bConnected.store(true);
	}
	catch (const std::exception& ex) {
		bConnected.store(false);
	}
}

Socket::~Socket()
{
		disconnect();
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
	if (!connected())
		return 0;

	int result = recv(m_hSocket, reinterpret_cast<char*>(pData), size, 0);  // probably not MSG_OOB?

	if (result == SOCKET_ERROR)
	{
		bConnected.store(false);
		return 0;
	}

	return static_cast<DWORD>(result);
}

void Socket::onReceive(void(*onReceive)(Socket*, const NET_MESSAGE, const void*))
{
	this->m_onReceive = onReceive;
	this->setCompletionRoutine(onCompletionRoutine);
}

void Socket::setCompletionRoutine(void(*completionRoutine)(DWORD, DWORD, LPWSAOVERLAPPED, DWORD))
{
	if (!connected())
		return;

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
	// safeguard
	if (m_hSocket == NULL)
		return;

	// We're canceling all pending operations, In case any are going on that might overwrite other resources.
	std::lock_guard<std::mutex> lock(m_mtWsaEvent);

	if (connected()) {
		shutdown(m_hSocket, SD_BOTH);
		bConnected.store(false);
		closesocket(m_hSocket);
		m_hSocket = NULL;
	}
}

