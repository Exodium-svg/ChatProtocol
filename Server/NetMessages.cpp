#include "NetMessages.h"

void OnLogin(Socket* pSocket, const NET_MSG_LOGIN* pMsg)
{
    const char* pUsername = (const char*)&pMsg->username;
    const char* pPassword = (const char*)&pMsg->password;

    ChatUser* pUser = Server::GetUser(pSocket->m_handle);

    if (!pUser) {
        pSocket->send(NET_RESULT(NetResult::NO_HANDLE, NET_ID_LOGIN));
        pSocket->disconnect();
        return;
    }

    // check if user exists in database? ( implement database logic )






    //we need a way to send result????
    //pSocket->send(NET_RESULT(NetResult::SUCCESS, NET_ID_LOGIN));
    pSocket->send(NET_MSG_HANDLE(pUser->m_hHandle));
}

void OnHandleRequest(Socket* pSocket, const NET_MSG_HANDLE* pMsg)
{
    if (pMsg->hHandle == NULL) {
        // allocate new handle;

        return;
    }

    // Get old handle?
}
