#pragma once
#include "netheader.h"

constexpr int USERNAME_LENGTH = 256;
constexpr int PASSWORD_LENGTH = 256;

constexpr uint16_t NET_ID_LOGIN = 0;
constexpr uint16_t NET_ID_ERROR = 1;
constexpr uint16_t NET_ID_HEARTBEAT = 2;
constexpr uint16_t NET_ID_HANDLE = 3;

DLL_SPEC enum NetResult : uint16_t {
	NOT_ALLOWED,
	NOT_EXIST,
	NO_HANDLE,
	SUCCESS,
};


#pragma pack(push, 1)  // Set the packing alignment to 1 byte

//TODO: this information could be used on the server as a hash to check whether someone is already in memory? NOTE: make sure to disconnect everything else.
struct DLL_SPEC NET_MSG_LOGIN : public NET_MESSAGE {
public:
	char username[USERNAME_LENGTH];
	char password[PASSWORD_LENGTH];

	NET_MSG_LOGIN(const char* pUsername, const char* pPassword);
};

struct DLL_SPEC NET_RESULT : public NET_MESSAGE {
public:
	const uint16_t nNetId;
	const uint16_t nResult;

	NET_RESULT(const NetResult result, const uint16_t id);
};

struct DLL_SPEC NET_MSG_HEART : public NET_MESSAGE {
public:
	const uint16_t nNumber;
	NET_MSG_HEART();
};

struct DLL_SPEC NET_MSG_HANDLE : public NET_MESSAGE {
public:
	const Handle hHandle;
	NET_MSG_HANDLE(const Handle hHandle);
};
#pragma pack(pop)  // Restore the default alignment