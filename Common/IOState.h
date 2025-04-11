#pragma once
#include "pch.h"
#include <WinSock2.h>
#include "IOCPConnection.h"

struct IOCPConnection;

enum DLL_SPEC EventType {
	Send,
	Header,
	Message,
	Disconnect,
};

struct DLL_SPEC IOState : public OVERLAPPED {
	EventType eType;
	WSABUF wsaBuff;
	unsigned long nMemLen;
	IOCPConnection* pConn;
};