#include "Server.h"

static IOCPState* iocp = nullptr;
static Handle m_nHandle{};
static std::vector<ChatUser> m_vUsers = std::vector<ChatUser>();

void Server::Initialize(Env& env)
{
	int32_t nThreads = env.GetInt("io.threads", 4);
	iocp = IOCP::InitializeIOCP(static_cast<uint32_t>(nThreads));
}

void Server::Deinitialize()
{
	IOCP::ShutdownIOCP(iocp);
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

IOCPState* Server::GetIOCP()
{
	return iocp;
}
