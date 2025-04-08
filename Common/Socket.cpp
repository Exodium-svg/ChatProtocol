#include "pch.h"
#include "Socket.h"

static bool initialized = false;
inline void ConnectToServer(Socket* pSocket, const char* pAddress, const uint16_t nPort) {
	sockaddr_in* pInAddr = &pSocket->inAddress;

	ZeroMemory(pInAddr, sizeof(sockaddr_in));

	pInAddr->sin_family = AF_INET;

	if (inet_pton(AF_INET, pAddress, &pInAddr->sin_addr) <= 0) {
		std::ostringstream sstream;
		sstream << "Invalid address: " << pAddress;
		throw std::runtime_error(sstream.str());
	}

	pInAddr->sin_port = htons(nPort);
	int nResult = connect(pSocket->hSocket, reinterpret_cast<sockaddr*>(&pSocket->inAddress), sizeof(sockaddr_in));

	if (nResult == SOCKET_ERROR) {
		std::ostringstream sstream;
		sstream << "Failed to connect to endpoint: " << pSocket->GetAddress() << ":" << pSocket->GetPort() << " error: " << WSAGetLastError();

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

	initialized = true;
}

void DeInitializeNetwork() { 
	initialized = false; 
	WSACleanup(); 
}

BOOL NetworkReady() { return initialized; }

inline void CloseSocket(Socket* pSocket) {
	if (pSocket->bConnected == FALSE && pSocket->hSocket == INVALID_SOCKET)
		return;

	pSocket->bConnected = FALSE;
	closesocket(pSocket->hSocket);
	pSocket->hSocket = INVALID_SOCKET;
}

inline void CheckApiError(DWORD dwError, Socket* pSocket) {
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
}
void CALLBACK onReceiveMessageRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags) {
	if (lpOverlapped->Pointer == nullptr)
		throw "Completion routine should always be called with the Overlapped pointer set.";

	Socket::IOState* pIoState = reinterpret_cast<Socket::IOState*>(lpOverlapped->Pointer);
	Socket* pSocket = pIoState->pSocket;
	
	if (WaitForSingleObject(pSocket->hMutex, INFINITE) == WAIT_ABANDONED)
		return;

	CheckApiError(dwError, pSocket);

	if (pSocket->bConnected == FALSE) {
		CloseSocket(pSocket);
		ReleaseMutex(pSocket->hMutex);
		return;
	}

	if (cbTransferred != pIoState->wsaBuff.len) {
		CloseSocket(pSocket);
		ReleaseMutex(pSocket->hMutex);
		return;
	}

	const NET_MESSAGE* pMsg = reinterpret_cast<NET_MESSAGE*>(pIoState->wsaBuff.buf);

	try {
		if (pSocket->onReceive == nullptr)
			throw std::runtime_error("Receive callback not set.");

		pSocket->onReceive(pSocket, pMsg);
	}
	catch (const std::exception& ex) {
		// do something?
	}

	if (pSocket->bConnected == FALSE) {
		CloseSocket(pSocket);
		ReleaseMutex(pSocket->hMutex);
		return;
	}

	pSocket->startCompletionRoutine(SockState::ReceiveHeader);
	ReleaseMutex(pSocket->hMutex);
}
void CALLBACK onReceiveHeaderRoutine(DWORD dwError, DWORD cbTransferred, LPWSAOVERLAPPED lpOverlapped, DWORD dwFlags)
{
	if (lpOverlapped->Pointer == nullptr)
		throw "Completion routine should always be called with the Overlapped pointer set.";

	Socket::IOState* pIoState = reinterpret_cast<Socket::IOState*>(lpOverlapped->Pointer);
	Socket* pSocket = pIoState->pSocket;

	if (WaitForSingleObject(pSocket->hMutex, INFINITE) == WAIT_ABANDONED)
		return;

	CheckApiError(dwError, pSocket);

	if (pSocket->bConnected == FALSE) {
		CloseSocket(pSocket);
		ReleaseMutex(pSocket->hMutex);
		return;
	}

	// We are out of sync.
	// Best to just not question it, However when a shutdown message is received it will be caught in this condition as well.
	if (cbTransferred < sizeof(NET_MESSAGE)) {
		CloseSocket(pSocket);
		ReleaseMutex(pSocket->hMutex);
		return;
	}

#ifdef _DEBUG
	if (sizeof(NET_MESSAGE) > pIoState->wsaBuff.len) {
		pSocket->bConnected = FALSE;
		ReleaseMutex(pSocket->hMutex);
		throw std::runtime_error("Invalid memory, not enough allocated");
	}
#endif

	const NET_MESSAGE* pHeader = reinterpret_cast<const NET_MESSAGE*>(pIoState->wsaBuff.buf);

	if (pHeader->length == 0) {
		CloseSocket(pSocket);
		ReleaseMutex(pSocket->hMutex);
		return;
	}

	// Do a memory pressure check to see if we should lower memory usage.
	if (pHeader->length > pIoState->nMemSize) {
		void* curMem = pIoState->wsaBuff.buf;
		pIoState->wsaBuff.buf = reinterpret_cast<char*>(realloc(curMem, pHeader->length));

		if (pIoState->wsaBuff.buf == nullptr) {
			// failed to allocate memory
			free(curMem);

			pIoState->wsaBuff.buf = reinterpret_cast<char*>(malloc(pHeader->length));
		}

		pIoState->nMemSize = pHeader->length;
	}
	
	pIoState->wsaBuff.len = pHeader->length;

	if (pSocket->bConnected == FALSE) {
		CloseSocket(pSocket);
		ReleaseMutex(pSocket->hMutex);
		return;
	}
	
	pSocket->startCompletionRoutine(SockState::ReceiveMessage);

	ReleaseMutex(pSocket->hMutex);
}

Socket::Socket(sockaddr_in address, SOCKET hSocket)
{
	onReceive = nullptr;
	this->hSocket = hSocket;
	ioState.pSocket = this;
	pOverlapped = new OVERLAPPED();
	pOverlapped->Pointer = &ioState;
	ioState.sockState = SockState::ReceiveHeader;
	ioState.wsaBuff.buf = reinterpret_cast<char*>(malloc(sizeof(NET_MESSAGE)));
	ioState.wsaBuff.len = sizeof(NET_MESSAGE);
	ioState.nMemSize = 0;
	inAddress = address;
	hMutex = CreateMutexA(NULL, false, NULL);
	hHandle = NULL;
	bConnected = TRUE;
}

Socket::Socket(const char* pAddress, const uint16_t nPort)
{
	this->hSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (hSocket == INVALID_SOCKET)
		throw std::runtime_error("Invalid socket? winsock2 intialized?");

	onReceive = nullptr;
	ioState.pSocket = this;
	pOverlapped = new OVERLAPPED();
	pOverlapped->Pointer = &ioState;
	ioState.sockState = SockState::ReceiveHeader;
	ioState.wsaBuff.buf = reinterpret_cast<char*>(malloc(sizeof(NET_MESSAGE)));
	ioState.wsaBuff.len = sizeof(NET_MESSAGE);
	ioState.nMemSize = 0;
	ZeroMemory(&inAddress, sizeof(inAddress));
	hMutex = CreateMutexA(NULL, false, NULL);
	hHandle = NULL;
	bConnected = TRUE;

	try {
		ConnectToServer(this, pAddress, nPort);
	}
	catch (const std::exception&) {
		bConnected = FALSE;
		return;
	}
}

Socket::~Socket()
{
	delete pOverlapped;
	free(ioState.wsaBuff.buf);
	CloseHandle(hMutex);
	closesocket(hSocket);
}

std::unique_ptr<char> Socket::GetAddress()
{
	std::unique_ptr<char> pAddrBuff = std::make_unique<char>(INET_ADDRSTRLEN);

	InetNtopA(AF_INET, &inAddress, pAddrBuff.get(), INET_ADDRSTRLEN);

	return std::move(pAddrBuff);
}

uint16_t Socket::GetPort() { return ntohs(inAddress.sin_port); }

DWORD Socket::send(const void* pBuff, DWORD dwSize)
{
	if (bConnected == FALSE)
		return NULL;

	if (WaitForSingleObject(hMutex, INFINITE) == WAIT_ABANDONED)
		return NULL;

	_WSABUF wsaBuff{};
	wsaBuff.buf = const_cast<char*>(reinterpret_cast<const char*>(pBuff));
	wsaBuff.len = dwSize;

	DWORD dwBytesSent;
	// turn into a routine so we check if it failed	
	int nResult = WSASend(hSocket, &wsaBuff, 1, &dwBytesSent, NULL, pOverlapped, NULL);

	if (nResult == SOCKET_ERROR) {
		bConnected = FALSE;
		ReleaseMutex(hMutex);
		return NULL;
	}

	if (dwBytesSent != dwSize) {
		bConnected = FALSE;
	}

	ReleaseMutex(hMutex);
	return dwBytesSent;
}

const DWORD Socket::receive(void* pBuff, DWORD dwSize)
{
	if (bConnected == FALSE)
		return NULL;

	int nResult = recv(hSocket, reinterpret_cast<char*>(pBuff), dwSize, NULL);

	if (nResult == SOCKET_ERROR) {
		bConnected = FALSE;
		return NULL;
	}

	if (nResult != dwSize) {
		bConnected = FALSE;
		return NULL;
	}

	return static_cast<DWORD>(nResult);
}

void Socket::setOnReceive(void(*onReceive)(Socket* pSocket, const NET_MESSAGE* pMsg)) { 
	this->onReceive = onReceive; 
	startCompletionRoutine();
}

void Socket::startCompletionRoutine(SockState eState)
{
	if (bConnected == FALSE)
		return;

	ioState.sockState = eState;

	DWORD dwMsgSize;
	DWORD dwFlags = NULL;

	int nResult;
	if (eState == SockState::ReceiveHeader) {
		dwFlags = MSG_PEEK;
		dwMsgSize = (DWORD)sizeof(NET_MESSAGE);
		nResult = WSARecv(hSocket, &ioState.wsaBuff, 1, &dwMsgSize, &dwFlags, pOverlapped, onReceiveHeaderRoutine);
	}
	else if (eState == SockState::ReceiveMessage) {
		dwMsgSize = ioState.wsaBuff.len;
		nResult = WSARecv(hSocket, &ioState.wsaBuff, 1, &dwMsgSize, &dwFlags, pOverlapped, onReceiveMessageRoutine);
	}
	else {
		throw std::runtime_error("Unable to set routine for state, Only receiveHeader and ReceiveMessage allowed.");
	}

	// If a operation is handled in Async, then the result code would be that it is pending.
	if (nResult == SOCKET_ERROR && WSAGetLastError() != WSA_IO_PENDING)
		bConnected = FALSE;
}

void Socket::disconnect()
{
	WaitForSingleObject(hMutex, INFINITE);
	
	if (bConnected) {
		bConnected = FALSE;
		shutdown(hSocket, SD_BOTH);
	}

	ioState.sockState = SockState::Disconnected;

	CloseSocket(this);
	//if (closesocket(hSocket) == SOCKET_ERROR) {
	//	std::stringstream ss;
	//	ss << "Failed to close socket error: " << WSAGetLastError();
	//	ReleaseMutex(hMutex);
	//	throw std::runtime_error(ss.str());
	//}

	hSocket = INVALID_SOCKET;
	ReleaseMutex(hMutex);
}
