#include "NetMessages.h"

void OnLogin(IOCPConnection* pConn, const NET_MSG_LOGIN* pMsg)
{
    const char* pUsername = (const char*)&pMsg->username;
    const char* pPassword = (const char*)&pMsg->password;
    
    ChatUser* pUser = Server::GetUser(pConn->hHandle);

    if (!pUser) {
        pConn->dispatchMsg(NET_RESULT(NetResult::NO_HANDLE, NET_ID_LOGIN));
        pConn->dispatchDisconnect();
        return;
    }

    // check if user exists in database? ( implement database logic )
     





    //we need a way to send result????
    //pSocket->send(NET_RESULT(NetResult::SUCCESS, NET_ID_LOGIN));
    pConn->dispatchMsg(NET_MSG_HANDLE(pUser->m_hHandle));
}

void OnHeartBeat(IOCPConnection* pConn)
{
    pConn->dispatchMsg(NET_MSG_HEART());
}

void OnHandleRequest(Socket* pSocket, const NET_MSG_HANDLE* pMsg)
{
    if (pMsg->hHandle == NULL) {
        // allocate new handle;

        return;
    }

    // Get old handle?
}
