#include "UiMessages.h"

void OnLogin(const UiLogin* pMsg)
{

    if (Application::GetUIState() != UIState::LOGIN)
        throw std::runtime_error("Unable to login as UIState is not on login.");

    Application::GetNetwork()->Dispatch((NET_MESSAGE*)new NET_MSG_LOGIN(pMsg->username, pMsg->password));
}

UiLogin::UiLogin(const char* pUsername, const char* pPassword)
    : UiMsg(UI_LOGIN_ID)
{
    memset(username, '\0', USERNAME_LEN);
    strcpy_s(username, USERNAME_LEN, pUsername);

    memset(password, '\0', PASSWORD_LEN);
    strcpy_s(password, PASSWORD_LEN, pPassword);
}