#pragma once
#include <vector>
#include "ChatUser.h"
#include <Env.h>
#include <IOCP.h>
#include <IOCPAccepter.h>
class Server
{
public:	
	static void Initialize(Env& env);
	static void Deinitialize();
	static void AddConnection(IOCPConnection* pConn);
	static IOCPConnection* GetConnection(Handle hHandle);
	//Memory leak city as don't clean up yippieeeee
	static ChatUser* AllocateUser();
	static ChatUser* GetUser(const Handle hHandle);
	static IOCPState* GetIOCP();
	static IOCPAccepter* GetIOICPAccepter();
};

