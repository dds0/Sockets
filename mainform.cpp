#include "mainform.h"
#include "ui_mainform.h"

#define BUFF_COEFF 2

MainForm::MainForm(QWidget* tmp, const IPv4& old, QWidget *parent) :
    startForm(tmp),
    QWidget(parent),
    ui(new Ui::MainForm)
{
    ui->setupUi(this);
    route = new IPv4(old);
    raw_data = new QByteArray();
    sourceFile = new QFile();

    setStartProperties();
    this->setWindowTitle("Потоковая передача аудио");

    socket = new QUdpSocket();
    timer = new QTimer();

    socket->bind(QHostAddress(route->getInfo().first), quint16(this->route->getPORTS().first));

    connect(socket, &QUdpSocket::readyRead, this, &MainForm::readPendingDatagrams);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));
}

MainForm::~MainForm()
{
    delete sourceFile;
    delete raw_data;
    delete buffer;
    delete socket;
    delete ui;
    delete startForm;
    delete audio;
    delete recievedFormat;
    delete packetManager;
}

void MainForm::setStartProperties()
{
    ui->From->setText(this->route->getInfo().first);
    ui->To->setText(this->route->getInfo().second);
    ui->PORT1->setText(QString::number(this->route->getPORTS().first));
    ui->PORT2->setText(QString::number(this->route->getPORTS().second));
    ui->packetSize->append("10000");
    ui->progressBar->hide();
    ui->textBrowser->append(getTime() + " Welcome!");
}

void MainForm::readPendingDatagrams()
{
    QHostAddress sender;
    quint16 senderPort;

    QByteArray raw_packet;
    raw_packet.resize(socket->pendingDatagramSize());
    socket->readDatagram(raw_packet.data(), raw_packet.size(), &sender, &senderPort);
    ui->textBrowser->append(getTime() + " Get Packet!");

    QAudioFormat* tmp_format = packetManager->setFormat(raw_packet.mid(raw_packet.size() - 14, 14), isPacket::YES);
    ++recievedPacket;

    if (!isInitializeFormat)
    {
        recievedFormat = tmp_format;
        isInitializeFormat = true;
        audio = new QAudioOutput(*recievedFormat);
        recievedSizePacket = raw_packet.size();
        audio->setBufferSize((raw_packet.size() - 14)*BUFF_COEFF);
        recievedPacket = 1;
    }
    else
    {
        if (!this->compareFormats(tmp_format, recievedFormat))
        {
            qDebug() << "Change format.";
            delete recievedFormat;
            delete audio;
            recievedFormat = tmp_format;
            audio = new QAudioOutput(*recievedFormat);
            recievedSizePacket = raw_packet.size();
            recievedPacket = 1;
        }
    }

    if (recievedSizePacket != raw_packet.size())
    {
        qDebug() << "Change recieved packet size";
        audio->setBufferSize((raw_packet.size() - 14)*BUFF_COEFF);
        recievedSizePacket = raw_packet.size();
        recievedPacket = 1;
    }

    if (recievedPacket == 1)
        whenStart = audio->start();
    else
        whenStart->write(raw_packet.mid(0, raw_packet.size() - 14));
}

void MainForm::slotTimerAlarm()
{
    int quantity_packets = qCeil((raw_data->size() + packetSize - 1)/(packetSize));

    if (currentPack == quantity_packets)
        timer->stop();
    else
    {
        QByteArray data;
        data = packetManager->sendPacket(currentPack, packetSize, raw_data);

        socket->writeDatagram(data, QHostAddress(this->route->getInfo().second), quint16(this->route->getPORTS().second));

        ui->textBrowser->append(getTime() + " Send Packet! [" + \
        QString::number(currentPack + 1) + "/" + QString::number(quantity_packets) + "]");
        ++currentPack;

        if (currentPack == quantity_packets)
            timer->stop();

        int progressValue = double(double(currentPack + 1)/double(quantity_packets))*100;
        ui->progressBar->setValue(progressValue);
    }
}

QString getTime()
{
    return "[" + QTime::currentTime().toString("HH:mm:ss") + "]";
}

void MainForm::on_Import_clicked()
{
    try
    {
        this->filename = QFileDialog::getOpenFileName(this, tr("Open WAV file."), "", tr("(*.wav)"));
        if (this->filename == "")
            throw std::runtime_error("Choose file.");

        this->getRawDataFromFile();
        ui->textBrowser->setText(getTime() + " Import file:" + this->filename + ".");

        if (isImport)
            delete packetManager;
        else
            isImport = true;
        packetManager = new PacketManager(raw_data->mid(0,44));

        QAudioBuffer *bf = new QAudioBuffer(*raw_data, *packetManager->get_ptr_format());
        this->set_Data_AudioBuffer(bf);

        ui->progressBar->show();
        ui->progressBar->setValue(0);

        ui->textBrowser->setText(getTime() + " Duration between packets: " + QString::number(fileDuration/(fileSize/packetSize)) + " ms.");
    }
    catch(std::exception&)
    {
        ui->textBrowser->setText(getTime() + " Can't open file: " + this->filename + ".");
    }
}

void MainForm::set_Data_AudioBuffer(QAudioBuffer *buffer)
{
    fileDuration = buffer->duration()/1000;
    fileSize = buffer->byteCount();
    qDebug() << "File duration: " << fileDuration;
    qDebug() << "File     size: " << fileSize;

    raw_data->clear();
    raw_data->append(buffer->data<char>(), buffer->byteCount());
    delete buffer;
}

void MainForm::getRawDataFromFile()
{
    sourceFile->setFileName(this->filename);
    sourceFile->open(QIODevice::ReadOnly);
    *raw_data = sourceFile->readAll();
    sourceFile->close();
}

bool MainForm::compareFormats(QAudioFormat *left, QAudioFormat *right)
{
    if (left->sampleSize() != right->sampleSize())
        return false;
    if (left->sampleRate() != right->sampleRate())
        return false;
    if (left->sampleType() != right->sampleType())
        return false;
    if (left->byteOrder() != right->byteOrder())
        return false;
    if (left->channelCount() != right->channelCount())
        return false;
    if (left->codec() != right->codec())
        return false;
    return true;
}

void MainForm::on_SetPacket_clicked()
{
    try
    {
        if (std::stoi(ui->packetSize->toPlainText().toStdString()) > 20000 || std::stoi(ui->packetSize->toPlainText().toStdString()) < 500)
            throw std::runtime_error("Choose another size packet.");

        ui->textBrowser->append(getTime() + " Set new size packet from: " + QString::number(packetSize + 14) + " to " + ui->packetSize->toPlainText() +".");
        packetSize = ui->packetSize->toPlainText().toInt() - 14;
    }
    catch(std::exception& ex)
    {
        if (ex.what() == std::string("invalid stoi argument"))
            QMessageBox::warning(this,"Oops...", QString("Invalid arguments."));
        else
            QMessageBox::warning(this,"Oops...", QString::fromStdString(ex.what()));
    }
}


void MainForm::on_Launch_clicked()
{
    if (!this->filename.isEmpty())
    {
        int time = fileDuration/(fileSize/packetSize) * 0.95;
        timer->start(time);
        ui->textBrowser->append(getTime() + " Start.");
        ui->SetPacket->setEnabled(false);
        ui->Import->setEnabled(false);
    }
    else
        ui->textBrowser->append(getTime() + " Need import file.");
}


void MainForm::on_Stop_clicked()
{
    if (!this->filename.isEmpty())
    {
        timer->stop();
        ui->textBrowser->append(getTime() + " Stop.");
    }
    else
        ui->textBrowser->append(getTime() + " Need import file.");


}


void MainForm::on_Restarting_clicked()
{
    if (!this->filename.isEmpty())
    {
        timer->stop();
        ui->textBrowser->append(getTime() + " Restart.");
        ui->SetPacket->setEnabled(true);
        ui->Import->setEnabled(true);
        this->currentPack = 0;
        ui->progressBar->setValue(0);
    }
    else
        ui->textBrowser->append(getTime() + " Need import file.");
}

void MainForm::add_to_output()
{
    qDebug() << "notify";
}

