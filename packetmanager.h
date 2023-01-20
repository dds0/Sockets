#ifndef PACKETMANAGER_H
#define PACKETMANAGER_H

#include <QByteArray>
#include <QAudioFormat>
#include <QPair>
#include <QAudioFormat>
#include <QDebug>

enum class isPacket
{
    YES = 0,
    NO = 1
};

class PacketManager
{
public:
    PacketManager(const QByteArray &header);
    PacketManager(const PacketManager& old);

    QByteArray sendPacket(qint64 currentP, qint64 packetSize, QByteArray* arr);
    QAudioFormat* recievedPacket(const QByteArray& arr);
    QAudioFormat *get_ptr_format();
    ~PacketManager();

    QAudioFormat* setFormat(const QByteArray &arr, isPacket what);

private:
    quint32 sampleSize, sampleRate;
    quint16 sampleType;
    char ByteOrder[4];
    QAudioFormat *format;
};

#endif // PACKETMANAGER_H
