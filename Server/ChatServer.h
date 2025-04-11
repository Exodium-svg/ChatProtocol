#pragma once
#include <iostream>
//#include <SocketAccepter.h>
#include <IOCPAccepter.h>
#include <NetworkStream.h>
#include <Env.h>
#include "Server.h"
#include "NetMessages.h"
#include <NetMessage.h>
#include <IOCP.h>

void OnConnect(IOCPConnection* pConn);
void OnMessage(Socket* pSocket, const NET_MESSAGE* pMsg);