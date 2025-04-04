#pragma once

#include <QtWidgets/QMainWindow>
#include <qmessagebox.h>
#include "ui_ChatClient.h"
#include "Application.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void onLoginAttempt() {
        QString qStrUsername = ui.login_username->text();
        QString qStrPassword = ui.login_password->text();

        // invalid string we ignore it
        if (qStrUsername.length() == 0 || qStrPassword.length() == 0)
            return; 

        std::string sUsername = qStrUsername.toStdString();
        std::string sPassword = qStrPassword.toStdString();

        InterfaceEventBus::Dispatch(new UiLogin(sUsername.c_str(), sPassword.c_str()));
        //Application::GetNetwork()->Dispatch((NET_MESSAGE*)new NET_MSG_LOGIN(sUsername.c_str(), sPassword.c_str()));
    }
    void setupCallbacks() {
        QObject::connect(ui.login_attempt, &QPushButton::clicked, this, &MainWindow::onLoginAttempt);
    }
    Ui::ChatClientClass ui;
};
