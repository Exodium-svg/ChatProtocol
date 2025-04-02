#pragma once
#include "netheader.h"


constexpr int USERNAME_LENGTH = 256;
constexpr int PASSWORD_LENGTH = 256;


constexpr uint16_t NET_ID_LOGIN = 0;
constexpr uint16_t NET_ID_ERROR = 1;
constexpr uint16_t NET_ID_HEARTBEAT = 2;

DLL_SPEC enum NetResult : uint16_t {
	NOT_ALLOWED,
	NOT_EXIST,
	NO_HANDLE,
	SUCCESS,
};


#pragma pack(push, 1)  // Set the packing alignment to 1 byte

struct DLL_SPEC NET_MSG_LOGIN : private NET_MESSAGE {
public:
	char username[USERNAME_LENGTH];
	char password[PASSWORD_LENGTH];

	NET_MSG_LOGIN(const char* pUsername, const char* pPassword);
};

struct DLL_SPEC NET_RESULT : private NET_MESSAGE {
public:
	const uint16_t nNetId;
	const uint16_t nResult;

	NET_RESULT(const NetResult result, const uint16_t id);
};

struct DLL_SPEC NET_MSG_HEART : private NET_MESSAGE {
public:
	NET_MSG_HEART();
};

#pragma pack(pop)  // Restore the default alignment