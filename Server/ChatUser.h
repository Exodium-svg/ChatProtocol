#pragma once
#include <Defines.h>
#include <Socket.h>
#include <string>
#include <memory>
//
//enum DLL_SPEC UserState {
//	Login,
//	LoggedIn,
//	Disconnected,
//};

DLL_SPEC struct ChatUser
{
public:
	ChatUser(const Handle hHandle, const char* pUsername);
	~ChatUser();
	//UserState m_eState;
	bool m_bConnected;
	const Handle m_hHandle;
	std::string m_pUsername;
};

