#ifndef UDPMULTICASTSERVER_H
#define UDPMULTICASTSERVER_H

#include "CommonSetting.h"
#include "shuakajiconfig.h"
#include "jiayouzhanconfig.h"
#include "deviceupgrade.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef Q_OS_LINUX
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#ifdef Q_OS_WIN32
#include <Ws2tcpip.h>
#endif

#define MCAST_RECV_PORT                  8888
#define MCAST_RECV_ADDR                  "224.0.0.88"
#define MCAST_SEND_PORT                  8889
#define MCAST_SEND_ADDR                  "224.0.0.89"
#define MCAST_INTERVAL                   1
#define MCAST_SEARCH_DEVICE              "Server###SearchDevice"
#define BUFF_SIZE                        1025


#define JiaYouZhanConfigBranch1
#define ShuaKaJiConfigBranch1
#define FaKaJiConfigBranch

namespace Ui {
class UdpMulticastServer;
}

class UdpMulticastServer : public QWidget
{
    Q_OBJECT

public:
    explicit UdpMulticastServer(QWidget *parent = 0);
    ~UdpMulticastServer();
    void tableSetAlignment();
    void tableWidgetSetting();

private slots:
    void on_btnSearchDevice_clicked();
    void on_btnConfigureDevice_clicked();
    void on_btnUpgradeDevice_clicked();

    void slotProcessPendingDatagrams();
    void slotSendConfigureInfo(QString ConfigureInfo);

    void on_tableWidget_cellClicked(int row, int column);
    void on_tableWidget_cellDoubleClicked(int row, int column);

private:
    Ui::UdpMulticastServer *ui;
    QTimer *ProcessDatagramTimer;

#ifdef Q_OS_LINUX
    struct sockaddr_in RecvMulticastAddr;//从组播接受消息
    struct sockaddr_in SendMulticastAddr;//用来给组播发送消息
    struct ip_mreq mreq;
    int RecvSocketFd;
    int SendSocketFd;
    socklen_t iSocketLen;
#endif
#ifdef Q_OS_WIN32
    struct sockaddr_in RecvMulticastAddr;
    struct sockaddr_in SendMulticastAddr;
    struct sockaddr_in RecvRemoteMulticastAddr;//从组播接受消息
    struct sockaddr_in SendRemoteMulticastAddr;//用来给组播发送消息
    struct ip_mreq mreq;
    SOCKET RecvSocketFd;
    SOCKET SendSocketFd;
    socklen_t iSocketLen;
#endif
    char ReceiveBuffer[BUFF_SIZE];
    char SendBuffer[BUFF_SIZE];

#ifdef ShuaKaJiConfigBranch
    ShuaKaJiConfig *shuakajiconfig;
#endif

#ifdef JiaYouZhanConfigBranch
    JiaYouZhanConfig *jiayouzhanconfig;
#endif

    DeviceUpgrade *device_upgrade;
    int SelectedRowIndex;
    int row_index;

    QSqlQuery query;
};
#endif // UDPMULTICASTSERVER_H
