#include "ChatServer.h"
void OnConnect(IOCPConnection* pConn) {
    ChatUser* pUser = Server::AllocateUser();

    //Set owner ship of the resource to the socket.
    pConn->hHandle = pUser->m_hHandle;
    
    Server::AddConnection(pConn);
    pConn->Listen();
    
}

void OnDisconnect(IOCPConnection* pConn) {

}

void OnMessage(IOCPConnection* pConn, const NET_MESSAGE* pMsg) {
    // Reason we are sending sockets in the function is for memory safety, If it stops existing then it will not get here. That's why we resolve chat user.
    ChatUser* pUser = Server::GetUser(pConn->hHandle);

    if (pUser == nullptr) {
        pConn->dispatchDisconnect();
        return;
    }

    switch (pMsg->id) {
        case NET_ID_LOGIN:
            OnLogin(pConn, static_cast<const NET_MSG_LOGIN*>(pMsg));
        break;
        case NET_ID_HEARTBEAT:
            OnHeartBeat(pConn);
        break;
        default:
            // Invalid message we block user here.
            std::unique_ptr<char> pAddress = pConn->GetAddress();
            Server::GetIOICPAccepter()->BlockAddress(pAddress.get());

            pConn->dispatchDisconnect();
        break;
    }

    // Log message sent?
}


// TODO: the server needs a way to clean up all disconnected sockets....
int main()
{
    Env env(FilePath("vars.env"));
    InitializeNetwork();

    Server::Initialize(env);
    IOCPState* pState = Server::GetIOCP();
    pState->onReceive = OnMessage;
    pState->onDisconnect = OnDisconnect;

    Server::GetIOICPAccepter()->SetReceive(OnConnect);

    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, GetCurrentProcessId());

    DWORD dwWaitResult = WaitForSingleObject(hProcess, INFINITE);

    CloseHandle(hProcess);

    return dwWaitResult;
}


