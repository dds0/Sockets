#ifndef MAINFORM_H
#define MAINFORM_H

#include <QWidget>
#include <ipv4.h>
#include <QUdpSocket>
#include <QHostAddress>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QFileDialog>
#include <QMediaPlayer>
#include <QAudioOutput>
#include <QAudioFormat>
#include <QNetworkDatagram>
#include <QByteArray>
#include <QPointer>
#include "packetmaker.h"
#include <QThread>
#include <QRandomGenerator>
#include <QMediaMetaData>
#include <QTimer>

namespace Ui {
class MainForm;
}

enum class FileType
{
    MP3 = 0,
    WAV = 1
};

QString getTime();

class MainForm : public QWidget
{
    Q_OBJECT

public:
    int DELETE = 0;
    IPv4 *route;
    explicit MainForm(QWidget* tmp, const IPv4& old, QWidget *parent = nullptr);
    ~MainForm();

    void setStartProperties();

public slots:

    void ReadingData();

private slots:

    void on_Import_clicked();

    void on_Launch_clicked();

    void on_Stop_clicked();

    void on_Restarting_clicked();

    void slotTimerAlarm();

    void on_SetPacket_clicked();

private:
    bool isImport = false;
    bool isReading = false;
    bool isFill = false;

    void CheckSetPlayer();
    void set_mediaHelper(FileType recieved_type);
    void set_PM();

    PacketMaker *PM;
    FileType type;
    QPointer<QFile> mediaHelper, loaderFile;
    QByteArray *raw_data;
    QUdpSocket *socket;
    Ui::MainForm *ui;
    QWidget *startForm;
    QString filename;
    QPointer<QMediaPlayer> player, recievedPackPlayer;
    QAudioOutput *audioOutput;
    qint64 fileSize, fileDuration;

    qint64 currentPack = 0, packetSize = 6000;
    int recievedPacks = 0;
    QTimer *timer;
};

#endif // MAINFORM_H
