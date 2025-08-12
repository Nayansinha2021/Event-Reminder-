#ifndef ADDEVENTDIALOG_H
#define ADDEVENTDIALOG_H

#include <QDialog>
#include <QDateTime>

namespace Ui {
class AddEventDialog;
}

class AddEventDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddEventDialog(QWidget *parent = nullptr);
    ~AddEventDialog();

    QString getEventName() const;
    QDateTime getEventDateTime() const;

private:
    Ui::AddEventDialog *ui;
};

#endif // ADDEVENTDIALOG_H
