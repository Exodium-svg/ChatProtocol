#include "NetworkEventBus.h"

void HandleMessage(Socket* pSocket, const NET_MESSAGE* pMsg)
{
    switch (pMsg->id) {
        // we ignore this one, as it is a response to our heartbeat.
    case NET_ID_HEARTBEAT:
        break;
    }
}

void NetworkEventBus::NetLoop()
{
    while (true) {
        std::lock_guard<std::mutex> lock(m_mQueueOut);

        if (!m_socket.bConnected) {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            //TODO fix me: This is broken, still have to fix this...
            reconnect();
            continue;
        }

        // make a time check, need to see whether we should send heartbeat.
        std::chrono::milliseconds msCurTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());

        // this is broken somehow?
        if (msCurTime > m_msLastBeat) {
            m_msLastBeat = msCurTime + std::chrono::milliseconds(10000);
            m_qOutMessage.push(new NET_MSG_HEART());
        }

        if (m_qOutMessage.empty()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            continue;
        }

        while (!m_qOutMessage.empty()) {
            NET_MESSAGE* pMsg = m_qOutMessage.front();
            m_qOutMessage.pop();

            DWORD dwSentBytes = m_socket.send(pMsg, pMsg->length);

            // Ensure data is actually sent.
            if (dwSentBytes != pMsg->length) {
                throw std::runtime_error(std::format("Not all packetData has been sent %d | %d have been sent.", dwSentBytes, pMsg->length));
            }

            delete pMsg;
        }
    }
}

NetworkEventBus::NetworkEventBus(const std::string& sAddress, const uint16_t nPort): m_socket(Socket(sAddress.c_str(), nPort))
{
    m_msLastBeat = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now().time_since_epoch());
    if (sAddress.empty())
        throw std::runtime_error("Invalid address, may not be empty.");

    m_qOutMessage = std::queue<NET_MESSAGE*>();

    m_socket.setOnReceive(HandleMessage);
    m_tNetMain = std::thread([this] {NetLoop(); });
}

bool NetworkEventBus::connected() { return m_socket.bConnected; }

void NetworkEventBus::reconnect()
{
    throw "not implemented";
    //m_socket.Reconnect();
}

void NetworkEventBus::Dispatch(NET_MESSAGE* pMsg)
{
    std::lock_guard<std::mutex> lock(m_mQueueOut);
    m_qOutMessage.push(pMsg);
}
