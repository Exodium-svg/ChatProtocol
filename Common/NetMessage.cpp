#include "pch.h"
#include "NetMessage.h"

NET_MSG_HEART::NET_MSG_HEART() :NET_MESSAGE(sizeof(NET_MSG_HEART), NET_ID_HEARTBEAT, 0), nNumber(0) {}

NET_RESULT::NET_RESULT(const NetResult result, const uint16_t id) : nNetId(id), nResult((uint16_t)result), NET_MESSAGE(sizeof(NET_RESULT), NET_ID_ERROR, 0) {}

NET_MSG_LOGIN::NET_MSG_LOGIN(const char* pUsername, const char* pPassword) 
	: NET_MESSAGE(sizeof(NET_MSG_LOGIN), NET_ID_LOGIN, 0) {
	if (!pUsername || !pPassword)
		throw std::runtime_error("Username or password is null");

	// Count the string including the null byte.
	size_t nLenUsername = strlen(pUsername) + 1;
	size_t nLenPassword = strlen(pPassword) + 1;

	if (nLenUsername > USERNAME_LENGTH) {
		throw std::runtime_error("Username length too long");
	}

	if (nLenPassword > PASSWORD_LENGTH) {
		throw std::runtime_error("Password length too long");
	}

	// Gotta make sure we don't accidently sent data we should not.
	ZeroMemory(&username, sizeof(username));
	ZeroMemory(&password, sizeof(password));

	memcpy_s(&username, sizeof(username), pUsername, nLenUsername);
	memcpy_s(&password, sizeof(password), pPassword, nLenPassword);
}

NET_MSG_HANDLE::NET_MSG_HANDLE(const Handle hHandle): NET_MESSAGE(sizeof(NET_MSG_HANDLE), NET_ID_HANDLE, 0), hHandle(hHandle){}