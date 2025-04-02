#pragma once
#include <atomic>
#include <Socket.h>
#include <NetMessage.h>
#include <mutex>
#include <queue>
#include <string>
#include <chrono>
class NetworkEventBus
{
private:
	std::chrono::milliseconds m_msLastBeat;
	std::atomic_bool m_bConnected;
	Socket m_socket;

	std::mutex m_mQueueOut;
	std::queue<NET_MESSAGE*> m_qOutMessage;
	std::thread m_tNetMain;

	void NetLoop();
public:
	NetworkEventBus(const std::string& sAddress, const uint16_t nPort);
	bool connected() const;
	bool reconnect();
	
	void Dispatch(NET_MESSAGE* pMsg);
};

