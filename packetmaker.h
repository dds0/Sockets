#ifndef PACKETMAKER_H
#define PACKETMAKER_H

#include <QString>
#include <QByteArray>
#include <QByteArrayList>
#include <QPair>
#include <QDebug>

class PacketMaker
{
public:
    PacketMaker();
    PacketMaker(const QString& filename, const QByteArray *arr);
    int getFileType() const;
    QByteArray getPacket(const QByteArray* arr, qint64 packetSize, qint64 currentPack);
private:
    QByteArray header;
    int FileType;
};

#endif // PACKETMAKER_H
