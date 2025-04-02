#pragma once
#include <queue>
#include <mutex>
#include <stdint.h>
#include <atomic>
#include <format>
#include "UiMessages.h"
#include "UiMsg.h"
class InterfaceEventBus
{
private:
	static std::mutex m_busMutex;
	static std::atomic_bool m_bActive;
	static std::thread m_tEventMain;
	static std::queue<UiMsg*> m_queue;
	static void HandleMessage(const UiMsg* pMsg);
public:
	static void Initialize();
	static void BusLoop();
	static void Dispatch(UiMsg* pMsg);
};

