#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow) {
    ui->setupUi(this);
    connect(ui->actionReset, SIGNAL(triggered()), ui->centralWidget, SLOT(reset()));
    connect(ui->actionConfiguration, SIGNAL(triggered()), ui->centralWidget, SIGNAL(openConfDialog()));
    //this->show();
}

MainWindow::~MainWindow() {
    delete ui;
}
