#ifndef MAINFORM_H
#define MAINFORM_H

#include <QWidget>
#include "ipv4.h"
#include <QUdpSocket>
#include <QHostAddress>
#include <QFileDialog>
#include <QAudioOutput>
#include <QAudioInput>
#include <QTime>
#include <QTimer>
#include <QFile>
#include <QByteArray>
#include <QAudioFormat>
#include <QAudioDecoder>
#include <QAudioBuffer>
#include <QBuffer>
#include <QDataStream>
#include <QAudioDeviceInfo>
#include <QMessageBox>
#include "packetmanager.h"
#include <QPointer>
#include <QtMath>
#include <chrono>
#include <stdio.h>
#include <time.h>
#include <QDataStream>

namespace Ui {
class MainForm;
}

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

    void readPendingDatagrams();

private slots:

    void slotTimerAlarm();

    void on_Import_clicked();

    void on_SetPacket_clicked();

    void on_Launch_clicked();

    void on_Stop_clicked();

    void on_Restarting_clicked();

private:

    bool isImport = false;
    bool isInitializeFormat = false;
    bool isAudioReady = false;
    bool isFirstFull = false;

    void set_Data_AudioBuffer(QAudioBuffer *buffer);
    void getRawDataFromFile();
    bool compareFormats(QAudioFormat *left, QAudioFormat *right);

    QFile *sourceFile;
    QByteArray *raw_data;
    QAudioBuffer *buffer;
    QUdpSocket *socket;
    Ui::MainForm *ui;
    QWidget *startForm;
    QString filename;
    QAudioOutput *audio;
    QIODevice *whenStart;
    qint64 fileSize, fileDuration;
    QAudioFormat *recievedFormat;
    PacketManager *packetManager;
    QBuffer *packet_storage;
    QBuffer *packet_one, *packet_two;

    qint64 currentByte = 0, packetSize = 9986, recievedByte = 0, recievedSizePacket = 0;
    QTimer *timer;

    clock_t start, end;
};

#endif // MAINFORM_H
