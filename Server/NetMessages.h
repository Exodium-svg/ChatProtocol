#pragma once
#include <Socket.h>
#include "Server.h"
#include <NetMessage.h>

void OnLogin(IOCPConnection* pConn, const NET_MSG_LOGIN* pMsg);
void OnHeartBeat(IOCPConnection* pConn);
void OnHandleRequest(Socket* pSocket, const NET_MSG_HANDLE* pMsg);
