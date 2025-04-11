#include "ChatServer.h"
void OnConnect(IOCPConnection* pConn) {
    ChatUser* pUser = Server::AllocateUser();

    //Set owner ship of the resource to the socket.
    pConn->hHandle = pUser->m_hHandle;
    pConn->dispatchMsg("Hello there");
    //pSocket->setOnReceive(OnMessage);
    pConn->Listen();
    
}

void OnMessage(IOCPConnection* pConn, const NET_MESSAGE* pMsg) {
    // Reason we are sending sockets in the function is for memory safety, If it stops existing then it will not get here. That's why we resolve chat user.
    switch (pMsg->id) {
        case NET_ID_LOGIN:
            OnLogin(pConn, static_cast<const NET_MSG_LOGIN*>(pMsg));
        break;
        case NET_ID_HEARTBEAT:
            // Find a way to limit this?
            //pSocket->send(NET_MSG_HEART());
        break;
        default:
            // block user?
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

    const std::string address = env.GetString("net.address", "0.0.0.0");
    const uint16_t nPort = env.GetInt("net.port", 25566);
    
    const char* pAddress = address.c_str();
    
    IOCPAccepter accepter(pState->hIOCP, pAddress, nPort, OnConnect);
    //SocketAccepter accepter(address.c_str(), port.c_str(), OnConnect);

    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, GetCurrentProcessId());

    DWORD dwWaitResult = WaitForSingleObject(hProcess, INFINITE);

    CloseHandle(hProcess);

    return dwWaitResult;
}


