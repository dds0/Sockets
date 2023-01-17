#include "ipv4.h"

IPv4::IPv4(const QString &inIP, const QString &outIP, const QString& portIN, const QString& portOUT)
{
    if (inIP.isEmpty() || portIN.isEmpty() || portOUT.isEmpty() || outIP.isEmpty())
        throw std::exception("Complete the fields.");

    QStringList listIN = inIP.split("."), listOUT = outIP.split(".");

    if (listIN.size() != 4 && listOUT.size() != 4 )
        throw std::exception("Complete the fields.");

    for (int i = 0; i < 4; ++i)
    {
        if (std::stoi(listIN[i].toStdString()) > 255 || std::stoi(listIN[i].toStdString()) < 0)
            throw std::exception("Check IPv4.");
        if (std::stoi(listOUT[i].toStdString()) > 255 || std::stoi(listOUT[i].toStdString()) < 0)
            throw std::exception("Check IPv4.");
    }

    if (std::stoi(portIN.toStdString()) > 65355 || std::stoi(portIN.toStdString()) < 0 || \
            std::stoi(portOUT.toStdString()) > 65355 || std::stoi(portOUT.toStdString()) < 0)
        throw std::exception("Check port number.");

    if (inIP != outIP && std::stoi(portIN.toStdString()) == std::stoi(portOUT.toStdString()))
        throw std::exception("Check port number.");

    this->inIP = inIP;
    this->outIP = outIP;
    this->portIN = portIN.toUInt();
    this->portOUT = portOUT.toUInt();
}

IPv4::IPv4()
{

}

IPv4::IPv4(const IPv4 &old)
{
    inIP = old.inIP;
    outIP = old.outIP;
    portIN = old.portIN;
    portOUT = old.portOUT;
}

QPair<QString, QString> IPv4::getInfo() const
{
    return qMakePair(this->inIP, this->outIP);
}

QPair<size_t, size_t> IPv4::getPORTS() const
{
    return qMakePair(this->portIN, this->portOUT);
}
