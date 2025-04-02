#include "Mainwindow.h"
#include "Application.h"
#include <Env.h>
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);

    Env env(FilePath("var.env"));

    Application::Initiate(env);


    MainWindow mainWindow;
    mainWindow.show();

    return application.exec();
}
