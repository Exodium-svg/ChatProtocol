#include "Server.h"

static IOCPState* iocp = nullptr;
static Handle m_nHandle{};
static IOCPAccepter* m_accepter = nullptr;
static std::vector<ChatUser> m_vUsers = std::vector<ChatUser>();
// Nothing owns a IOCP connection, it is it's own entity
static std::vector<IOCPConnection*> m_vConns = std::vector<IOCPConnection*>();
void Server::Initialize(Env& env)
{
	int32_t nThreads = env.GetInt("io.threads", 4);
	iocp = IOCP::InitializeIOCP(static_cast<uint32_t>(nThreads));
	const std::string address = env.GetString("net.address", "0.0.0.0");
	const uint16_t nPort = env.GetInt("net.port", 25566);

	const char* pAddress = address.c_str();

	m_accepter = new IOCPAccepter(iocp->hIOCP, pAddress, nPort, nullptr);
}

void Server::Deinitialize()
{
	IOCP::ShutdownIOCP(iocp);
}

IOCPConnection* Server::GetConnection(Handle hHandle)
{
	for (size_t i = 0; m_vConns.size() > i; i++)
		if (m_vConns.at(i)->hHandle == hHandle)
			return m_vConns.at(i);

	return nullptr;
}

void Server::AddConnection(IOCPConnection* pConn) {
	bool bAdded = false;
	for (size_t i = 0; m_vConns.size() > i; i++) {
		if (nullptr == m_vConns.at(i)) {
			m_vConns[i] = pConn;
			bAdded = true;
		}
	}

	if (!bAdded)
		m_vConns.push_back(pConn);
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

IOCPAccepter* Server::GetIOICPAccepter()
{
	return m_accepter;
}
