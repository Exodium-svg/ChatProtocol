#pragma once
#include <iostream>
#include <SocketAccepter.h>
#include <NetworkStream.h>
#include <Env.h>
#include "Server.h"
#include "NetMessages.h"
#include <NetMessage.h>

void OnConnect(Socket* pSocket);
void OnMessage(Socket* pSocket, const NET_MESSAGE* pMsg);