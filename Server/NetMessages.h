#pragma once
#include <Socket.h>
#include "Server.h"
#include <NetMessage.h>

void OnLogin(Socket* pSocket, const NET_MSG_LOGIN* pMsg);

