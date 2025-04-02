#include "ChatServer.h"
void OnConnect(Socket* pSocket) {
    pSocket->onReceive(OnMessage);

    ChatUser* pUser = Server::AllocateUser();

    pSocket->m_handle = pUser->m_hHandle;
}

void OnMessage(Socket* pSocket, const NET_MESSAGE header, const void* pData) {
    // Reason we are sending sockets in the function is for memory safety, If it stops existing then it will not get here. That's why we resolve chat user.
    switch (header.id) {
        case NET_ID_LOGIN:
            OnLogin(pSocket, reinterpret_cast<const NET_MSG_LOGIN*>(pData));
        break;
    }

  
}



int main()
{
    Env env(FilePath("vars.env"));
    InitializeNetwork();

    Server::Initialize(env);


    const std::string address = env.GetString("net.address", "0.0.0.0");
    const std::string port = env.GetString("net.port", "25566");

    SocketAccepter accepter(address.c_str(), port.c_str(), OnConnect);

    
    Sleep(10000000000);
    return 0;
}


