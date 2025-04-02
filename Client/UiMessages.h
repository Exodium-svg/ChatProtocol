#pragma once
#include "Application.h"
#include "UiMsg.h"
#include "NetMessage.h"
#include "InterfaceEventBus.h"					
constexpr uint16_t USERNAME_LEN = 256;
constexpr uint16_t PASSWORD_LEN = 256;

constexpr uint16_t UI_LOGIN_ID = 0;

class UiLogin : public UiMsg {
public:
	UiLogin(const char* pUsername, const char* pPassword);
	char username[USERNAME_LEN];
	char password[PASSWORD_LEN];
};




void OnLogin(const UiLogin* pMsg);