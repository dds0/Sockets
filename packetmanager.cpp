#include "packetmanager.h"

PacketManager::PacketManager(const QByteArray& header)
{
    format = setFormat(header, isPacket::NO);
}

PacketManager::PacketManager(const PacketManager &old)
{
    this->sampleSize = old.sampleSize;
    this->sampleRate = old.sampleRate;
    this->sampleType = old.sampleType;

    int i = 0;
    while (i != 4)
    {
        ByteOrder[i] = old.ByteOrder[i];
        ++i;
    }

    this->format = old.format;
}

QByteArray PacketManager::sendPacket(qint64 currentB, qint64 packetSize, QByteArray *arr)
{
    QByteArray data;
    data = arr->mid(currentB, packetSize);

    data.append(arr->mid(28,4)); //sampleSize
    data.append(arr->mid(24,4)); //sampleRate
    data.append(arr->mid(34,2)); //sampleType
    data.append(arr->mid(0,4));  //ByteOrder
    return data;
}

QAudioFormat* PacketManager::recievedPacket(const QByteArray& arr)
{
    QAudioFormat *sendedFormat = this->setFormat(arr.mid(arr.size() - 14, 14), isPacket::YES);
    return sendedFormat;
}

QAudioFormat *PacketManager::get_ptr_format()
{
    return format;
}

PacketManager::~PacketManager()
{
    delete format;
}

QAudioFormat* PacketManager::setFormat(const QByteArray &arr, isPacket what = isPacket::YES)
{
    QAudioFormat *new_format = new QAudioFormat();

    quint32 temp_32;
    quint16 temp_16;
    char hh[4];
    int i = 0;

    new_format->setChannelCount(1);
    new_format->setCodec("audio/pcm");

    switch(what)
    {
    case (isPacket::NO):

        //qint16 temp_avg;
        //memcpy(&temp_avg, arr.mid(34,2), sizeof(quint16));
        //qDebug() << " BITS PER SEMPL:" << temp_avg;

        memcpy(&sampleSize, arr.mid(28,4), sizeof(quint32));
        sampleSize /= 1000;
        if (sampleSize > 32)
            sampleSize = 32;
        new_format->setSampleSize(sampleSize);
        qDebug() <<"Sample size:" << (sampleSize);

        memcpy(&sampleRate, arr.mid(24,4), sizeof(quint32));
        if (sampleRate == 22050)
           sampleRate /=2;
        new_format->setSampleRate(sampleRate);
        qDebug() <<"Sample rate:" << sampleRate;

        memcpy(&sampleType, arr.mid(34,2), sizeof(quint16));
        new_format->setSampleType(sampleType == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);
        qDebug() <<"Sample type:" << new_format->sampleType();

        while (i != 4)
        {
            ByteOrder[i] = arr.at(i);
            ++i;
        }
        if (memcmp(&ByteOrder, "RIFF", 4) == 0)
            new_format->setByteOrder(QAudioFormat::LittleEndian);
        else
            new_format->setByteOrder(QAudioFormat::BigEndian);
        qDebug() <<"Byte order:" << new_format->byteOrder();

        break;
    case (isPacket::YES):

        memcpy(&temp_32, arr.mid(0, 4), sizeof(quint32));
        temp_32 /= 1000;
        if (temp_32 > 32)
            temp_32 = 32;
        new_format->setSampleSize(temp_32);

        memcpy(&temp_32, arr.mid(4,4), sizeof(quint32));
        if (temp_32 == 22050)
           temp_32 /=2;
        new_format->setSampleRate(temp_32);

        memcpy(&temp_16, arr.mid(8,2), sizeof(quint16));
        new_format->setSampleType(temp_16 == 8 ? QAudioFormat::UnSignedInt : QAudioFormat::SignedInt);

        while (i != 4)
        {
            hh[i] = arr.at(10 + i);
            ++i;
        }
        if (memcmp(&hh, "RIFF", 4) == 0)
            new_format->setByteOrder(QAudioFormat::LittleEndian);
        else
            new_format->setByteOrder(QAudioFormat::BigEndian);
        break;
    }

    return new_format;
}

