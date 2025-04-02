#include "Server.h"


static Handle m_nHandle{};
static std::vector<ChatUser> m_vUsers = std::vector<ChatUser>();

void Server::Initialize(Env& env)
{
}

ChatUser* Server::AllocateUser()
{
	m_vUsers.emplace_back(m_nHandle++, "No name");

	return &m_vUsers.back();
}

ChatUser* Server::GetUser(const Handle hHandle)
{
	for (ChatUser& user : m_vUsers) {
		if (user.m_hHandle == hHandle)
			return &user;
	}

	return nullptr;
}
