#include "pch.h"
#include "IOCP.h"

inline void CleanupState(IOState* pState) {
	WSABUF wsaBuff = pState->wsaBuff;

	if (wsaBuff.buf != nullptr) {
		free(wsaBuff.buf);
		delete pState;
	}
}

inline void CleanupSocket(IOState* pState) {
	IOCPConnection* pConn = pState->pConn;

	closesocket(pConn->hSocket);
	pConn->bConnected = FALSE;

	CleanupState(pState);
}

inline BOOL ParseHeader(IOState* pState) {
	WSABUF* pWsaBuf = &pState->wsaBuff;
	const NET_MESSAGE* pHeader = reinterpret_cast<const NET_MESSAGE*>(pWsaBuf);

	//TODO: make a flag for chunked.
	// NO MESSAGE SHOULD BE THIS LONG.
	if (pHeader->length > 16000)
		return FALSE;


	if (pHeader->length > pWsaBuf->len) {
		void* pOldMem = pWsaBuf->buf;

		void* pNewMem = realloc(pOldMem, pHeader->length);

		if (nullptr == pNewMem) {
			pWsaBuf->buf = reinterpret_cast<char*>(malloc(pHeader->length));
			pWsaBuf->len = pHeader->length;
			free(pOldMem);
		}
		else {
			pWsaBuf->buf = reinterpret_cast<char*>(pNewMem);
			pWsaBuf->len = pHeader->length;
		}
	}

	return TRUE;
}

inline void HandleMessage(IOCPConnection* pConn, const IOState* pState, IOCPState* pIOCPState) {
	const NET_MESSAGE* pMsg = reinterpret_cast<const NET_MESSAGE*>(pState->wsaBuff.buf);

	try {
		pIOCPState->onReceive(pConn, pMsg);
	}
	catch (const std::exception&) {
		// handle exception?
	}
}

DWORD __stdcall IOCP::IOCPWorkerThread(LPVOID lpParam)
{
	IOCPState* pIOCPState = reinterpret_cast<IOCPState*>(lpParam);
	
	while (true) {
		DWORD dwBytesTransfered;
		ULONG_PTR pCompletionKey;
		LPOVERLAPPED pOverlapped;

		BOOL bResult = GetQueuedCompletionStatus(pIOCPState->hIOCP, &dwBytesTransfered, &pCompletionKey, &pOverlapped, INFINITE);

		if (nullptr == pOverlapped || FALSE == bResult) {
			continue; // graceful shutdown or error
		}

		IOState* pState = reinterpret_cast<IOState*>(pOverlapped);
		IOCPConnection* pConn = pState->pConn;

		if (0 > dwBytesTransfered) {
			CleanupSocket(pState);
			continue;
		}

		EventType type = pState->eType;

		switch (type) {
		case EventType::Header:
			if (ParseHeader(pState) == FALSE) {
				CleanupSocket(pState);
				continue;
			}
			break;
		case EventType::Message:
			HandleMessage(pConn, pState, pIOCPState);
			break;
		case EventType::Disconnect:
			CleanupSocket(pState);
			continue;
		case EventType::Send:
			CleanupState(pState);
			continue;
		default:
			// something went wrong?
			break;
		}

		if (EventType::Header != type)
			CleanupState(pState);

		DWORD dwFlags = NULL;

		if (EventType::Header == type)
			dwFlags = MSG_PEEK;

		int nResult = WSARecv(pConn->hHandle, &pState->wsaBuff, 1, nullptr, &dwFlags, pOverlapped, nullptr);

		if (nResult != NO_ERROR && WSA_IO_PENDING != WSAGetLastError())
			CleanupSocket(pState);
	}
}

DLL_SPEC IOCPState* IOCP::InitializeIOCP(uint32_t nThreads)
{
	IOCPState* pState = new IOCPState();

	pState->hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);

	for (unsigned int i = 0; i < nThreads; i++) {
		HANDLE hThread = CreateThread(
			nullptr,
			0,
			IOCP::IOCPWorkerThread,
			pState,
			0,
			0
		);

		if (hThread == nullptr) {
			throw std::runtime_error("Failed to create threads for IOCP");
		}

		std::wstringstream sstream;
		sstream << L"IOCP: " << i;

		HRESULT hResult = SetThreadDescription(hThread, sstream.str().c_str());

		if (FAILED(hResult)) {
			throw std::runtime_error("Failed to name thread for IOCP");
		}
	}

	return pState;
}
DLL_SPEC void IOCP::RegisterSocket(IOCP_t hIOCP, Socket* pSocket) { pSocket->bindToIOCP(hIOCP); }
DLL_SPEC void IOCP::ShutdownIOCP(IOCP_t hIOCP) { PostQueuedCompletionStatus(hIOCP, 0, 0, nullptr); }
