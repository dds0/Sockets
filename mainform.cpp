#include "mainform.h"
#include "ui_mainform.h"

MainForm::MainForm(QWidget* tmp, const IPv4& old, QWidget *parent) :
    startForm(tmp),
    QWidget(parent),
    ui(new Ui::MainForm)
{
    ui->setupUi(this);
    route = new IPv4(old);
    audioOutput = new QAudioOutput(this);
    loaderFile = new QFile();
    socket = new QUdpSocket();
    timer = new QTimer();

    socket->bind(QHostAddress(route->getInfo().first), quint16(this->route->getPORTS().first));

    connect(socket, &QUdpSocket::readyRead, this, &MainForm::ReadingData);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerAlarm()));

    this->setWindowTitle("Потоковая передача аудио");
    setStartProperties();
}

MainForm::~MainForm()
{
    delete raw_data;
    delete route;
    delete socket;
    delete ui;
    delete startForm;
    delete player;
    delete audioOutput;
    delete timer;
    delete mediaHelper;
    delete recievedPackPlayer;
}

void MainForm::setStartProperties()
{
    ui->From->setText(this->route->getInfo().first);
    ui->To->setText(this->route->getInfo().second);
    ui->PORT1->setText(QString::number(this->route->getPORTS().first));
    ui->PORT2->setText(QString::number(this->route->getPORTS().second));
    ui->packetSize->append("6000");
    ui->progressBar->hide();
    ui->textBrowser->append(getTime() + " Welcome!");
}

void MainForm::ReadingData()
{

    if (DELETE == 1)
        qDebug() <<"start";

    QHostAddress sender;
    quint16 senderPort;

    if (!isReading)
        this->CheckSetPlayer();

    recievedPackPlayer->setSource(QUrl());

    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());
    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);
    ui->textBrowser->append(getTime() + " Get Packet!");

    ++recievedPacks;

    qDebug() << "Sender is: " << sender.toString();
    qDebug() << "Sender port is: " << senderPort;
    qDebug() << "Received packet"<<recievedPacks<<"size:" << buffer.size() << "-" \
    << char(buffer.at(0)) << char(buffer.at(1))<< char(buffer.at(2));

    this->set_mediaHelper(FileType(int(buffer.at(buffer.size() - 1))));

    mediaHelper->open(QIODevice::WriteOnly);
    if (mediaHelper->isOpen())
    {
        int before = mediaHelper->size();
        mediaHelper->write(buffer);
        if (before != mediaHelper->size())
            qDebug() << "Recieved data saved.";

        mediaHelper->close();
    }
    else
        ui->textBrowser->append(getTime() + " Cannot write recieved packet!");

    recievedPackPlayer->setSource(QUrl::fromLocalFile(mediaHelper->fileName()));
    recievedPackPlayer->play();
}

void MainForm::on_Import_clicked()
{
    if (isImport)
        delete PM;

    try
    {
        this->filename = QFileDialog::getOpenFileName(this, tr("Open sound file."), "", tr("(*.mp3 *.wav)"));
        if (this->filename.isEmpty())
            throw std::exception("Choose file.");

        this->CheckSetPlayer();

        ui->progressBar->setValue(0);
        loaderFile->setFileName(this->filename);

        loaderFile->open(QIODevice::ReadOnly);
        if (loaderFile->isOpen())
        {
            if (isFill)
            {
                raw_data->clear();
                *raw_data = loaderFile->readAll();
            }
            else
            {
                raw_data = new QByteArray(loaderFile->readAll());
                isFill = true;
            }
            loaderFile->close();
            qDebug() << "File size is: " << raw_data->size();

            this->fileSize = raw_data->size();
            this->currentPack = 0;
            ui->progressBar->show();
        }
        else
            throw std::exception("Cannot open file.");

        this->isImport = true;

        this->set_PM();
        player->setSource(QUrl::fromLocalFile(filename));
        recievedPacks = 0;
        ui->SetPacket->setEnabled(true);

    }
    catch (std::exception&)
    {
        ui->textBrowser->append(getTime() + " Can't open file: " + this->filename + ".");
    }
}


void MainForm::on_Launch_clicked()
{
        ++DELETE;
    if (!this->filename.isEmpty())
    {
        if (mediaHelper && mediaHelper->isOpen())
            mediaHelper->close();
        qint64 tmp = this->fileDuration/(fileSize/packetSize);
        //timer->start(tmp + QRandomGenerator::global()->bounded(qint64(-0.2*tmp), qint64(0.2*tmp)));
        timer->start(tmp);
        qDebug()<<"1 packet per ms: +-" << tmp;
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
        if (mediaHelper && mediaHelper->isOpen())
            mediaHelper->close();
    }
    else
        ui->textBrowser->append(getTime() + " Need import file.");
}


void MainForm::on_Restarting_clicked()
{
    if (!this->filename.isEmpty())
    {
        timer->stop();
        this->currentPack = 0;
        ui->progressBar->setValue(0);
        ui->textBrowser->append(getTime() + " Restart.");
        ui->SetPacket->setEnabled(true);
        ui->Import->setEnabled(true);
        if (mediaHelper && mediaHelper->isOpen())
            mediaHelper->close();
    }
    else
        ui->textBrowser->append(getTime() + " Need import file.");
}

void MainForm::slotTimerAlarm()
{
    QByteArray data;
    data = this->PM->getPacket(raw_data, packetSize, currentPack);

    int all_packets;
    if (this->type == FileType::MP3)
        all_packets = qCeil((raw_data->size() + packetSize - 2)/(packetSize - 1));
    else if (this->type == FileType::WAV)
        all_packets = qCeil((raw_data->size() + packetSize - 2)/(packetSize - 1));

    socket->writeDatagram(data, QHostAddress(this->route->getInfo().second), quint16(this->route->getPORTS().second));

    qDebug() << "Send  a  packet"<<currentPack+1<<"size:" << data.size() << "-" \
    << char(data.at(0)) << char(data.at(1))<< char(data.at(2));
    ui->textBrowser->append(getTime() + " Send Packet! [" + \
    QString::number(currentPack + 1) + "/" + QString::number(all_packets) + "]");
    ++currentPack;

    if (currentPack == all_packets)
        timer->stop();

    int progressValue = double(double(currentPack + 1)/double(all_packets))*100;
    ui->progressBar->setValue(progressValue);
}

QString getTime()
{
    return "[" + QTime::currentTime().toString("HH:mm:ss") + "]";
}

void MainForm::on_SetPacket_clicked()
{
    try
    {
        if (std::stoi(ui->packetSize->toPlainText().toStdString()) > 20000 || std::stoi(ui->packetSize->toPlainText().toStdString()) < 500)
            throw std::exception("Choose another size packet.");

        ui->textBrowser->append(getTime() + " Set new size packet from: " + QString::number(packetSize) + " to " + ui->packetSize->toPlainText() +".");
        packetSize = ui->packetSize->toPlainText().toInt();
    }
    catch(std::exception& ex)
    {
        if (ex.what() == std::string("invalid stoi argument"))
            QMessageBox::warning(this,"Oops...", QString("Invalid arguments."));
        else
            QMessageBox::warning(this,"Oops...", QString::fromStdString(ex.what()));
    }
}

void MainForm::CheckSetPlayer()
{
    if (!this->isImport)
    {
        if (player)
            delete player;
        player = new QMediaPlayer(this);
        player->setAudioOutput(audioOutput);

        if (recievedPackPlayer)
            delete recievedPackPlayer;
        recievedPackPlayer = new QMediaPlayer(this);
        recievedPackPlayer->setAudioOutput(audioOutput);

        connect(player, &QMediaPlayer::durationChanged, this, [&](qint64 dur) {
            if (this->isImport)
            {
                qDebug()<<"Duration this track: " << dur;
                fileDuration = dur;
                ui->textBrowser->append(getTime() + " Delay between transmitted packets: " + \
                QString::number(this->fileDuration/(fileSize/packetSize)) + " ms.");
                isImport = false;
            }
        });

        isReading = true;
    }
}

void MainForm::set_PM()
{
    this->PM = new PacketMaker(this->filename, this->raw_data);
    type = FileType(PM->getFileType());
}

void MainForm::set_mediaHelper(FileType recieved_type)
{
    if (mediaHelper)
    {
        if (mediaHelper->isOpen())
            mediaHelper->close();
        mediaHelper->remove();
    }
    else
        mediaHelper = new QFile();

    if (recieved_type == FileType::MP3)
        mediaHelper->setFileName("service_file.mp3");
    else if (recieved_type == FileType::WAV)
        mediaHelper->setFileName("service_file.wav");

    mediaHelper->close();
}


