#include "addeventdialog.h"
#include "ui_addeventdialog.h"

AddEventDialog::AddEventDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddEventDialog)
{
    ui->setupUi(this);
    ui->dateTimeEdit->setDateTime(QDateTime::currentDateTime());
}

AddEventDialog::~AddEventDialog()
{
    delete ui;
}

QString AddEventDialog::getEventName() const
{
    return ui->lineEdit->text();
}

QDateTime AddEventDialog::getEventDateTime() const
{
    return ui->dateTimeEdit->dateTime();
}
