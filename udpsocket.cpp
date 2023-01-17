#include "udpsocket.h"

Socket::Socket(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Socket)
{
    ui->setupUi(this);
    socket = new QUdpSocket(this);
}

Socket::~Socket()
{
    delete ui;
}
