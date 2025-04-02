#include "Application.h"
UIState Application::m_state = UIState::LOGIN;
NetworkEventBus* Application::m_networkEventBus;
static bool bRunning = false;
UIState Application::GetUIState() { return m_state; }

void Application::SetUIState(const UIState& state) { m_state = state; }

void Application::Initiate(const Env& env)
{
	if (bRunning)
		throw std::runtime_error("Application already initialized!");

	bRunning = true;
	InitializeNetwork();

	const std::string address = env.GetString("network.address", "127.0.0.1");
	const uint16_t port = env.GetInt("network.port", 25566);

	InterfaceEventBus::Initialize();
	m_networkEventBus = new NetworkEventBus(address, port);
}

NetworkEventBus* Application::GetNetwork()
{
	return m_networkEventBus;
}

