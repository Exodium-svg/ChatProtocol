#include "pch.h"
#include "IOCP.h"

__forceinline void CleanupState(IOState* pState) {
	WSABUF wsaBuff = pState->wsaBuff;

	if (wsaBuff.buf != nullptr) {
		free(wsaBuff.buf);
		delete pState;
	}
}

__forceinline void CleanupSocket(IOState* pState) {
	IOCPConnection* pConn = pState->pConn;

	pConn->close();
	pConn->bConnected = false;

	CleanupState(pState);
}

__forceinline BOOL ProcessHeader(IOState* pState) {
	WSABUF* pWsaBuf = &pState->wsaBuff;
	const NET_MESSAGE* pHeader = reinterpret_cast<const NET_MESSAGE*>(pWsaBuf->buf);

	//TODO: make a flag for chunked.
	// NO MESSAGE SHOULD BE THIS LONG.
	if (pHeader->length > 16000)
		return FALSE;

	if (pHeader->length > pState->nMemLen) {
		void* pOldMem = pWsaBuf->buf;
		void* pNewMem = realloc(pOldMem, pHeader->length);

		if (nullptr == pNewMem) {
			pWsaBuf->buf = reinterpret_cast<char*>(malloc(pHeader->length));
			pWsaBuf->len = pHeader->length;
			pState->nMemLen = pWsaBuf->len;
			free(pOldMem);
		}
		else {
			pWsaBuf->buf = reinterpret_cast<char*>(pNewMem);
			pWsaBuf->len = pHeader->length;
		}
	}
	else
		pState->wsaBuff.len = pHeader->length;

	return TRUE;
}

inline void ProcessMessage(IOCPConnection* pConn, const IOState* pState, IOCPState* pIOCPState) {
	const NET_MESSAGE* pMsg = reinterpret_cast<const NET_MESSAGE*>(pState->wsaBuff.buf);

	try {
		pIOCPState->onReceive(pConn, pMsg);
	}
	catch (const std::exception&) {
		// Create exception event handler.
	}
}

DWORD __stdcall IOCP::IOCPWorkerThread(LPVOID lpParam)
{
	IOCPState* pIOCPState = reinterpret_cast<IOCPState*>(lpParam);

	while (pIOCPState->bAlive) {
		DWORD dwBytesTransfered;
		ULONG_PTR pCompletionKey;
		LPOVERLAPPED pOverlapped;

		BOOL bResult = GetQueuedCompletionStatus(pIOCPState->hIOCP, &dwBytesTransfered, &pCompletionKey, &pOverlapped, INFINITE);

		if (nullptr == pOverlapped || FALSE == bResult) {
			continue; //error or shutdown
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
			if (ProcessHeader(pState) == FALSE) {
				CleanupSocket(pState);
				continue;
			}
			break;
		case EventType::Message:
			ProcessMessage(pConn, pState, pIOCPState);
			break;
		case EventType::Disconnect:
			CleanupSocket(pState);
			pIOCPState->onDisconnect(pConn);
			continue;
		case EventType::Send:
			pIOCPState->nBytesSent += dwBytesTransfered;
			CleanupState(pState);
			continue;
		default:
			// something went wrong?
			break;
		}

		pIOCPState->nBytesReceived += dwBytesTransfered;

		DWORD dwFlags = NULL;

		// set the state correct to receive a header.
		if (EventType::Header == type) {
			dwFlags = MSG_PEEK;
			pState->wsaBuff.len = sizeof(NET_MESSAGE);
		}

		int nResult = WSARecv(pConn->hHandle, &pState->wsaBuff, 1, nullptr, &dwFlags, pOverlapped, nullptr);

		if (NO_ERROR != nResult && WSA_IO_PENDING != WSAGetLastError())
			CleanupSocket(pState);
	}

	return NO_ERROR;
}

DLL_SPEC IOCPState* IOCP::InitializeIOCP(uint32_t nThreads)
{
	IOCPState* pState = new IOCPState();
	ZeroMemory(pState, sizeof(IOCPState));

	pState->hIOCP = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
	pState->bAlive = true;
	pState->nThreads = nThreads;

	for (unsigned int i = 0; i < nThreads; i++) {
		HANDLE hThread = CreateThread(
			nullptr,
			0,
			IOCP::IOCPWorkerThread,
			pState,
			0,
			0
		);

		if (nullptr == hThread) {
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
DLL_SPEC void IOCP::ShutdownIOCP(IOCPState* pIOCP) {
	pIOCP->bAlive = false;

	for(uint32_t i = 0; i < pIOCP->nThreads; i++)
		PostQueuedCompletionStatus(pIOCP->hIOCP, 0, 0, nullptr);
}
