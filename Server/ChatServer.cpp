#include "ChatServer.h"
void OnConnect(Socket* pSocket) {
    pSocket->onReceive(OnMessage);

    ChatUser* pUser = Server::AllocateUser();
    //Set owner ship of the resource to the socket.
    pSocket->m_handle = pUser->m_hHandle;
    pSocket->send(NET_MSG_HEART());
    //pSocket->disconnect();
}

void OnMessage(Socket* pSocket, const NET_MESSAGE header, const void* pData) {
    // Reason we are sending sockets in the function is for memory safety, If it stops existing then it will not get here. That's why we resolve chat user.
    switch (header.id) {
        case NET_ID_LOGIN:
            OnLogin(pSocket, reinterpret_cast<const NET_MSG_LOGIN*>(pData));
        break;
        case NET_ID_HEARTBEAT:
            // Find a way to limit this?
            pSocket->send(NET_MSG_HEART());
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


    const std::string address = env.GetString("net.address", "0.0.0.0");
    const std::string port = env.GetString("net.port", "25566");

    SocketAccepter accepter(address.c_str(), port.c_str(), OnConnect);

    HANDLE hProcess = OpenProcess(SYNCHRONIZE, FALSE, GetCurrentProcessId());

    DWORD dwWaitResult = WaitForSingleObject(hProcess, INFINITE);

    CloseHandle(hProcess);

    return dwWaitResult;
}


