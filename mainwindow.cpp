#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addeventdialog.h"
#include <QFile>
#include <QTextStream>
#include <QMessageBox>
#include <QDateTime>
#include <QBrush>
#include <QTimer>
#include <QDir>
#include <QStandardPaths>
#include <QProcess>
#include <QSettings>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Tray icon
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setIcon(windowIcon());
    trayIcon->setToolTip("Event Reminder");
    connect(trayIcon, &QSystemTrayIcon::activated, this, &MainWindow::trayIconActivated);

    // Tray menu actions
    QAction *showAction = new QAction(tr("Show"), this);
    connect(showAction, &QAction::triggered, this, &MainWindow::on_actionShow_MainWindow_triggered);
    QAction *exitAction = new QAction(tr("Exit"), this);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction(showAction);
    trayMenu->addAction(exitAction);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();

    // Timer to check reminders
    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, &MainWindow::checkReminders);
    checkTimer->start(60000); // every 60 seconds
    // Also do an immediate check every 10 seconds at startup for responsiveness
    QTimer::singleShot(10000, this, &MainWindow::checkReminders);

    loadEventsFromFile();
    refreshEventHighlight();
    updateEventList();

    // Set autostart menu checked state
    ui->actionToggle_Autostart->setChecked(isAutostartEnabled());
}

MainWindow::~MainWindow()
{
    saveEventsToFile();
    delete ui;
}

void MainWindow::loadEventsFromFile()
{
    events.clear();
    QFile file("events.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.trimmed().isEmpty()) continue;
        QStringList parts = line.split('|');
        if (parts.size() == 2) {
            Event e;
            e.name = parts[0];
            e.dateTime = QDateTime::fromString(parts[1], Qt::ISODate);
            if (e.dateTime.isValid()) events.append(e);
        }
    }
    file.close();
}

void MainWindow::saveEventsToFile()
{
    QFile file("events.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (const auto &e : events) {
        out << e.name << "|" << e.dateTime.toString(Qt::ISODate) << "\n";
    }
    file.close();
}

void MainWindow::updateEventList()
{
    ui->listWidget->clear();
    QString filter = ui->searchEdit->text().trimmed();
    QList<Event> filtered;
    for (const auto &e : events) {
        if (filter.isEmpty() || e.name.contains(filter, Qt::CaseInsensitive)) {
            filtered.append(e);
        }
    }

    // Sort
    if (ui->sortCombo->currentIndex() == 0) {
        std::sort(filtered.begin(), filtered.end(), [](const Event &a, const Event &b){
            return a.dateTime < b.dateTime;
        });
    } else {
        std::sort(filtered.begin(), filtered.end(), [](const Event &a, const Event &b){
            return a.dateTime > b.dateTime;
        });
    }

    for (const auto &e : filtered) {
        QListWidgetItem *item = new QListWidgetItem(
            e.name + " - " + e.dateTime.toString("yyyy-MM-dd hh:mm"));
        ui->listWidget->addItem(item);
    }
    refreshEventHighlight();
}

void MainWindow::refreshEventHighlight()
{
    QDateTime now = QDateTime::currentDateTime();
    for (int i = 0; i < ui->listWidget->count(); ++i) {
        // Find corresponding event by text match (safe because list built from filtered sorted list)
        QString text = ui->listWidget->item(i)->text();
        // Extract datetime from item text (last 16 chars)
        QString dt = text.right(16);
        QDateTime d = QDateTime::fromString(dt, "yyyy-MM-dd hh:mm");
        if (!d.isValid()) continue;
        if (d < now && d.secsTo(now) <= 24*3600) {
            ui->listWidget->item(i)->setBackground(QBrush(Qt::red));
        } else if (d < now) {
            ui->listWidget->item(i)->setBackground(QBrush(Qt::lightGray));
        } else if (d.secsTo(now) <= 0) {
            ui->listWidget->item(i)->setBackground(QBrush(Qt::yellow));
        } else {
            ui->listWidget->item(i)->setBackground(QBrush(Qt::white));
        }
    }
}

void MainWindow::on_addButton_clicked()
{
    AddEventDialog dialog(this);
    if (dialog.exec() == QDialog::Accepted) {
        Event e;
        e.name = dialog.getEventName();
        e.dateTime = dialog.getEventDateTime();
        events.append(e);
        updateEventList();
        saveEventsToFile();
    }
}

void MainWindow::on_removeButton_clicked()
{
    int row = ui->listWidget->currentRow();
    if (row >= 0) {
        // Map row back to events: use filter+sort same as updateEventList
        QString text = ui->listWidget->item(row)->text();
        QString dt = text.right(16);
        QDateTime d = QDateTime::fromString(dt, "yyyy-MM-dd hh:mm");
        QString name = text.left(text.size()-19).trimmed();
        // find matching event
        for (int i = 0; i < events.size(); ++i) {
            if (events[i].name == name && events[i].dateTime.toString("yyyy-MM-dd hh:mm") == d.toString("yyyy-MM-dd hh:mm")) {
                events.removeAt(i);
                break;
            }
        }
        updateEventList();
        saveEventsToFile();
    }
}

void MainWindow::on_removeMultipleButton_clicked()
{
    QList<QListWidgetItem*> selected = ui->listWidget->selectedItems();
    QList<QPair<QString, QString>> toRemove;
    for (QListWidgetItem *item : selected) {
        QString text = item->text();
        QString dt = text.right(16);
        QString name = text.left(text.size()-19).trimmed();
        toRemove.append(qMakePair(name, dt));
    }
    // remove from events by matching name+dt
    for (auto p : toRemove) {
        for (int i = events.size()-1; i >=0; --i) {
            if (events[i].name == p.first && events[i].dateTime.toString("yyyy-MM-dd hh:mm") == p.second) {
                events.removeAt(i);
            }
        }
    }
    updateEventList();
    saveEventsToFile();
}

void MainWindow::on_searchEdit_textChanged(const QString &text)
{
    Q_UNUSED(text);
    updateEventList();
}

void MainWindow::on_sortCombo_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateEventList();
}

void MainWindow::checkReminders()
{
    QDateTime now = QDateTime::currentDateTime();
    for (const auto &e : events) {
        qint64 secs = now.secsTo(e.dateTime);
        if (secs <= 300 && secs > 0) { // within 5 minutes
            trayIcon->showMessage("Upcoming: " + e.name, "Starts at " + e.dateTime.toString("yyyy-MM-dd hh:mm"), QSystemTrayIcon::Information, 10000);
        } else if (secs <= 0 && secs > -60) { // due within last minute or now
            trayIcon->showMessage("Due now: " + e.name, e.dateTime.toString("yyyy-MM-dd hh:mm"), QSystemTrayIcon::Critical, 15000);
        }
    }
    refreshEventHighlight();
}

bool MainWindow::isAutostartEnabled()
{
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    return settings.contains("EventReminder");
#else
    QString path = autostartDesktopPath();
    return QFile::exists(path);
#endif
}

void MainWindow::setAutostartEnabled(bool enable)
{
#ifdef Q_OS_WIN
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);
    if (enable) {
        settings.setValue("EventReminder", QCoreApplication::applicationFilePath().replace('/','\\'));
    } else {
        settings.remove("EventReminder");
    }
#else
    QString path = autostartDesktopPath();
    if (enable) {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "[Desktop Entry]\n";
            out << "Type=Application\n";
            out << "Name=EventReminder\n";
            out << "Exec=" << QCoreApplication::applicationFilePath() << "\n";
            out << "X-GNOME-Autostart-enabled=true\n";
            file.close();
        }
    } else {
        QFile::remove(path);
    }
#endif
}

QString MainWindow::autostartDesktopPath()
{
    QString configHome = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
    QDir dir(configHome);
    dir.mkpath("autostart");
    return dir.filePath("autostart/eventreminder.desktop");
}

void MainWindow::on_actionToggle_Autostart_triggered()
{
    bool enable = ui->actionToggle_Autostart->isChecked();
    setAutostartEnabled(enable);
    QMessageBox::information(this, tr("Auto-start"), enable ? tr("Enabled") : tr("Disabled"));
}

void MainWindow::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if (reason == QSystemTrayIcon::Trigger) {
        this->show();
        this->raise();
        this->activateWindow();
    }
}

void MainWindow::on_actionShow_MainWindow_triggered()
{
    this->show();
    this->raise();
    this->activateWindow();
}
