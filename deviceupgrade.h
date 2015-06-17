#ifndef DEVICEUPGRADE_H
#define DEVICEUPGRADE_H

#include "sendfileclient.h"

namespace Ui {
class DeviceUpgrade;
}

class DeviceUpgrade : public QWidget
{
    Q_OBJECT

public:
    explicit DeviceUpgrade(QWidget *parent = 0);
    ~DeviceUpgrade();

private slots:
    void on_btnOpen_clicked();
    void on_btnUpgrade_clicked();

    void setSendPBar(qint64 size);
    void updateSendPBar(qint64 size);
    void sendFinsh();
    void slotCloseConnection();

public:
    Ui::DeviceUpgrade *ui;
    QString ServerIP;

    qint64 fileSize;
    qint64 sendBytes;
    SendFileClient *send;
};

#endif // DEVICEUPGRADE_H
