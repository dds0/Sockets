#include "packetmaker.h"

PacketMaker::PacketMaker()
{

}

PacketMaker::PacketMaker(const QString& filename, const QByteArray *arr)
{
    if (filename.contains(".wav"))
    {
        header.resize(40);
        for (int var = 0; var < 40; ++var) {
            header[var] = arr->at(var);
        }
        FileType = 1;
    }
    else if (filename.contains(".mp3"))
    {
        FileType = 0;
    }
}

int PacketMaker::getFileType() const
{
    return this->FileType;
}

QByteArray PacketMaker::getPacket(const QByteArray *arr, qint64 packetSize, qint64 currentPack)
{
    QByteArray data;
    qint64 temp = packetSize * currentPack;

    if (currentPack != 0 && FileType == 1)
    {
        data = arr->mid(temp - 40, packetSize - 1);
        data.append(char(FileType));
        for (int var = 0; var < 40; ++var) {
            data[var] = header.at(var);
        }
    }
    else
    {
        data = arr->mid(temp, packetSize - 1);
        data.append(char(FileType));
    }

    return data;
}
