#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QList>
#include <QDateTime>
#include <QSystemTrayIcon>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

struct Event {
    QString name;
    QDateTime dateTime;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void loadEventsFromFile();
    void saveEventsToFile();
    void refreshEventHighlight();
    void scheduleChecks();

private slots:
    void on_addButton_clicked();
    void on_removeButton_clicked();
    void on_removeMultipleButton_clicked();
    void on_searchEdit_textChanged(const QString &text);
    void on_sortCombo_currentIndexChanged(int index);
    void checkReminders();
    void on_actionToggle_Autostart_triggered();
    void trayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void on_actionShow_MainWindow_triggered();

private:
    Ui::MainWindow *ui;
    QList<Event> events;
    void updateEventList();
    QSystemTrayIcon *trayIcon;
    QTimer *checkTimer;
    bool isAutostartEnabled();
    void setAutostartEnabled(bool enable);
    QString autostartDesktopPath();
};

#endif // MAINWINDOW_H
