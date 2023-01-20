#ifndef IPV4_H
#define IPV4_H
#include <QString>
#include <QException>
#include <exception>
#include <QPair>
#include <iostream>
#include <QObject>
#include <QStringList>
#include <QDebug>

class IPv4
{
public:
    IPv4(const QString& inIP, const QString& outIP, const QString& portIN, const QString& portOUT);
    IPv4();
    IPv4(const IPv4& old);
    QPair<QString, QString> getInfo() const ;
    QPair<std::size_t, std::size_t> getPORTS() const;
private:
    QString inIP, outIP;
    std::size_t portIN, portOUT;
};

#endif // IPV4_H
