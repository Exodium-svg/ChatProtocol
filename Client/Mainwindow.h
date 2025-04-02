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
        QMessageBox::information(this, "Login", "Login button clicked!");
        
        Application::GetNetwork()->Dispatch((NET_MESSAGE*)new NET_MSG_HEART());
    }
    void setupCallbacks() {
        QObject::connect(ui.login_attempt, &QPushButton::clicked, this, &MainWindow::onLoginAttempt);
    }
    Ui::ChatClientClass ui;
};
