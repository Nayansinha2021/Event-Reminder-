#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QApplication::setApplicationName("EventReminder");
    QApplication::setOrganizationName("EventReminderOrg");
    MainWindow w;
    w.show();
    return a.exec();
}
