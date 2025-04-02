#include "InterfaceEventBus.h"

std::queue<UiMsg*> InterfaceEventBus::m_queue = std::queue<UiMsg*>();
std::mutex InterfaceEventBus::m_busMutex;
std::atomic_bool InterfaceEventBus::m_bActive = false;
std::thread InterfaceEventBus::m_tEventMain;

void InterfaceEventBus::Initialize()
{
	if (m_bActive)
		throw "Inferface EventBus already exists";

	m_bActive = true;
	m_tEventMain = std::thread(InterfaceEventBus::BusLoop);
}

void InterfaceEventBus::BusLoop()
{
	while (m_bActive) {
		std::lock_guard<std::mutex> lock(m_busMutex);
		if (m_queue.empty()) {
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			continue;
		}

		UiMsg* pMsg = m_queue.front();
		m_queue.pop();

		HandleMessage(pMsg);
		delete pMsg;
	}

	m_bActive = false;
}

void InterfaceEventBus::HandleMessage(const UiMsg* pMsg)
{
	switch (pMsg->m_nId) {
		case UI_LOGIN_ID:
			OnLogin(reinterpret_cast<const UiLogin*>(pMsg));
		break;
		default:
			throw std::format("Unknown UiMessage ID has been dispatched %d", pMsg->m_nId);
		break;
	}
}

void InterfaceEventBus::Dispatch(UiMsg* pMsg)
{
	std::lock_guard<std::mutex> lock(m_busMutex);
	m_queue.push(pMsg);
}
