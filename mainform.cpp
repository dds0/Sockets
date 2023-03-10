#include "mainform.h"
#include "ui_mainform.h"

MainForm::MainForm(QWidget* tmp, const IPv4& old, QWidget *parent) :
    startForm(tmp),
    QWidget(parent),
    ui(new Ui::MainForm)
{
    ui->setupUi(this);
    route = new IPv4(old);
    raw_data = new QByteArray();
    sourceFile = new QFile();
    packet_storage = new QBuffer();
    packet_one = new QBuffer();
    packet_two = new QBuffer();

    setStartProperties();
    this->setWindowTitle("Потоковая передача аудио");

    socket = new QUdpSocket();
    timer = new QTimer();

    socket->bind(QHostAddress(route->getInfo().first), quint16(this->route->getPORTS().first));
    socket->setSocketOption(QAbstractSocket::LowDelayOption, 1);

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
    delete packet_storage;
    delete packet_one, packet_two;
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
    recievedByte += raw_packet.size();
    ++DELETE;
    int BufferSize = 45'000;

    if (!isInitializeFormat)
    {
        recievedFormat = tmp_format;
        isInitializeFormat = true;
        audio = new QAudioOutput(*recievedFormat);
        recievedSizePacket = raw_packet.size();
        recievedByte = 0;

        isAudioReady = false;
        audio->setBufferSize(BufferSize);
    }
    else
    {
        if (!this->compareFormats(tmp_format, recievedFormat) || recievedSizePacket != raw_packet.size())
        {
            qDebug() << "Change format.";

            delete audio;
            delete recievedFormat;

            recievedFormat = tmp_format;
            audio = new QAudioOutput(*recievedFormat);
            recievedSizePacket = raw_packet.size();
            recievedByte = 0;

            isAudioReady = false;
            isFirstFull = false;
            audio->setBufferSize(BufferSize);
        }
    }


    if (!isFirstFull)
    {
        packet_one->open(QIODevice::WriteOnly | QIODevice::Append);
        packet_one->write(raw_packet.mid(0, raw_packet.size() - 14));
        packet_one->close();

        if (!isAudioReady && packet_one->size() >= BufferSize/3)
        {
            whenStart = audio->start();
            whenStart->open(QIODevice::ReadWrite | QIODevice::Append);
            isAudioReady = true;
        }

        if (packet_one->size() >= BufferSize/3)
        {
            packet_one->open(QIODevice::ReadWrite);
            whenStart->write(packet_one->data());
            packet_one->close();
            packet_one->buffer().clear();
            isFirstFull = true;
        }
    }
    else
    {
        packet_two->open(QIODevice::WriteOnly | QIODevice::Append);
        packet_two->write(raw_packet.mid(0, raw_packet.size() - 14));
        packet_two->close();

        if (packet_two->size() >= BufferSize/3)
        {
            packet_two->open(QIODevice::ReadWrite);
            whenStart->write(packet_two->data());
            packet_two->close();
            packet_two->buffer().clear();

            isFirstFull = false;
        }
    }


    //if (recievedByte <= BufferSize/2)
    //{
    //    //start = clock();

    //    packet_storage->open(QIODevice::WriteOnly | QIODevice::Append);
    //    packet_storage->write(raw_packet.mid(0, raw_packet.size() - 14));
    //    packet_storage->close();
    //    //end = clock();
    //}
    //else if (isAudioReady)
    //{
    //    //double seconds = (double)(end - start) / CLOCKS_PER_SEC;
    //    //qDebug() << "SECONDS: " << seconds;
    //    whenStart->write(raw_packet.mid(0, raw_packet.size() - 14));
    //}

    //if (recievedByte >= BufferSize/2 && !isAudioReady)
    //{
    //    whenStart = audio->start();
    //    whenStart->open(QIODevice::ReadWrite | QIODevice::Append);
    //    packet_storage->open(QIODevice::ReadWrite);
    //    whenStart->write(packet_storage->data());
    //    packet_storage->buffer().clear();
    //    packet_storage->close();

    //    isAudioReady = true;
    //}

    //qDebug() <<"(recieved pack)"<<recievedByte/recievedSizePacket << ": (free bytes)"<< audio->bytesFree();
    qDebug() <<"(recieved pack)"<<DELETE << ": (free bytes)"<< audio->bytesFree();
}

void MainForm::slotTimerAlarm()
{
    int quantity_packets = qCeil((raw_data->size() + packetSize - 1)/(packetSize));
    int curr_pack = currentByte/packetSize;

    if (curr_pack == quantity_packets)
        timer->stop();
    else
    {
        QByteArray data;
        data = packetManager->sendPacket(currentByte, packetSize, raw_data);

        socket->writeDatagram(data, QHostAddress(this->route->getInfo().second), quint16(this->route->getPORTS().second));

        ui->textBrowser->append(getTime() + " Send Packet! [" + \
        QString::number(curr_pack + 1) + "/" + QString::number(quantity_packets) + "]");
        currentByte += packetSize;

        if (currentByte/packetSize == quantity_packets)
            timer->stop();

        int progressValue = double(double(curr_pack + 1)/double(quantity_packets))*100;
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
        ui->textBrowser->append(getTime() + " Import file:" + this->filename + ".");

        if (isImport)
            delete packetManager;
        else
            isImport = true;
        packetManager = new PacketManager(raw_data->mid(0,44));

        QAudioBuffer *bf = new QAudioBuffer(*raw_data, *packetManager->get_ptr_format());
        this->set_Data_AudioBuffer(bf);

        ui->progressBar->show();
        ui->progressBar->setValue(0);
        qDebug() << "raw data for recieve:" << raw_data->size();
    }
    catch(std::exception&)
    {
        ui->textBrowser->append(getTime() + " Can't open file: " + this->filename + ".");
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

        if (std::stoi(ui->packetSize->toPlainText().toStdString()) % 100 != 0)
            throw std::runtime_error("100 fold packet.");

        ui->textBrowser->append(getTime() + " Set new size packet from: " + QString::number(packetSize + 14) + " to " + ui->packetSize->toPlainText() +".");
        packetSize = ui->packetSize->toPlainText().toInt() - 14;
        if (!ui->Import->isEnabled())
        {
            int time = double(fileDuration)/(double(fileSize)/double(packetSize)) + 1 - int(packetSize/2750)*5;
            timer->start(time);
            ui->textBrowser->append(getTime() + " Duration between packets: " + QString::number(time) + " ms.");
        }
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
        qDebug() << double(fileDuration)/(double(fileSize)/double(packetSize));
        int time = double(fileDuration)/(double(fileSize)/double(packetSize)) + 1 - int(packetSize/2750)*5;
        qDebug() << double(fileDuration)/(double(fileSize)/double(packetSize)) << " "<< time;
        ui->textBrowser->append(getTime() + " Duration between packets: " + QString::number(time) + " ms.");

        timer->start(time);
        ui->textBrowser->append(getTime() + " Start.");
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
        ui->Import->setEnabled(true);
        this->currentByte = 0;
        ui->progressBar->setValue(0);
    }
    else
        ui->textBrowser->append(getTime() + " Need import file.");
}

