#pragma once
#include <thread>
#include <Socket.h>
#include <Env.h>
#include "NetworkEventbus.h"
#include "InterfaceEventBus.h"

enum UIState {
	LOGIN,
	REGISTER,
};

class Application
{
public:
	static UIState GetUIState();
	static void SetUIState(const UIState& state);
	static void Initiate(const Env& env);
	static NetworkEventBus* GetNetwork();
private:
	static UIState m_state;
	static NetworkEventBus* m_networkEventBus;
};

