#include "Mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
    setupCallbacks();
}

MainWindow::~MainWindow()
{}
