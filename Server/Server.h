#pragma once
#include <vector>
#include "ChatUser.h"
#include <Env.h>
DLL_SPEC class Server
{

public:	
	static void Initialize(Env& env);
	//Memory leak city as don't clean up yippieeeee
	static ChatUser* AllocateUser();
	static ChatUser* GetUser(const Handle hHandle);
};

