#ifndef SOCKET_H
#define SOCKET_H

#include <QUdpSocket>
#include <QWidget>

QT_BEGIN_NAMESPACE
namespace Ui { class Socket; }
QT_END_NAMESPACE

class Socket : public QWidget
{
    Q_OBJECT
public:
    explicit Socket(QWidget *parent = nullptr);
    ~Socket();
private:
    Ui::Socket *ui;
    QUdpSocket *socket;
    bool isGetData;
};

#endif // SOCKET_H
