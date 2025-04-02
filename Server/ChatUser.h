#pragma once
#include <Defines.h>
#include <Socket.h>
#include <string>
#include <memory>


DLL_SPEC struct ChatUser
{
public:
	ChatUser(const Handle hHandle, const char* pUsername);
	~ChatUser();
	bool m_bConnected;
	const Handle m_hHandle;
	std::string m_pUsername;
};

