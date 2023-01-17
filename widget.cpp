#include "widget.h"
#include "./ui_widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    this->setWindowTitle("Потоковая передача аудио");
}

Widget::~Widget()
{
    delete ui;
}


void Widget::on_pushButton_clicked()
{
    try
    {
        IPv4 tmp(ui->IPv4_IN->toPlainText(), ui->IPv4_OUT->toPlainText(), ui->portIN->toPlainText(), ui->portOUT->toPlainText());
        this->mainForm = new MainForm(this, tmp);

        this->mainForm->show();
        this->hide();
    }
    catch (std::exception& ex)
    {
        if (ex.what() == std::string("invalid stoi argument"))
            QMessageBox::warning(this,"Oops...", QString("Invalid arguments."));
        else
            QMessageBox::warning(this,"Oops...", QString::fromStdString(ex.what()));
    }
}


