#include "udpmulticastserver.h"
#include "ui_udpmulticastserver.h"

//如果仅仅是想向一个组播组发送数据，而不要接受数据，那么可不用加入组播组，而直接通过sendto向组播组发送数据

UdpMulticastServer::UdpMulticastServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UdpMulticastServer)
{
    ui->setupUi(this);

    row_index = 0;
    SelectedRowIndex = -1;
    tableWidgetSetting();

#ifdef ShuaKaJiConfigBranch
    this->setWindowTitle(tr("组合认证刷卡主机配置升级工具"));
    shuakajiconfig = new ShuaKaJiConfig;
    connect(shuakajiconfig,SIGNAL(signalSendConfigureInfo(QString)),this,SLOT(slotSendConfigureInfo(QString)));
#endif

#ifdef JiaYouZhanConfigBranch
    this->setWindowTitle(tr("零拷加油配置升级工具"));
    jiayouzhanconfig = new JiaYouZhanConfig;
    connect(jiayouzhanconfig,SIGNAL(signalSendConfigureInfo(QString)),this,SLOT(slotSendConfigureInfo(QString)));
#endif

#ifdef FaKaJiConfigBranch
    this->setWindowTitle(tr("组合认证发卡主机升级工具"));
    ui->btnConfigureDevice->setVisible(false);
#endif

    device_upgrade = new DeviceUpgrade;

    ProcessDatagramTimer = new QTimer(this);
    ProcessDatagramTimer->setInterval(400);
    connect(ProcessDatagramTimer,SIGNAL(timeout()),this,SLOT(slotProcessPendingDatagrams()));

#ifdef Q_OS_LINUX
    RecvSocketFd = socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);
    if(RecvSocketFd < 0){
        perror("socket()");
        return;
    }

    SendSocketFd = socket(AF_INET,SOCK_DGRAM,IPPROTO_IP);
    if(SendSocketFd < 0){
        perror("socket()");
        ::close(RecvSocketFd);
        return;
    }

    if(fcntl(RecvSocketFd,F_SETFL,O_NONBLOCK) < 0){
        perror("fcntl()");
        ::close(RecvSocketFd);
        ::close(SendSocketFd);
        return;
    }

    //用来从组播组接收消息
    memset(&RecvMulticastAddr,0,sizeof(struct sockaddr_in));
    RecvMulticastAddr.sin_family = AF_INET;
    RecvMulticastAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    RecvMulticastAddr.sin_port = htons(MCAST_RECV_PORT);

    //用来给组播组发送消息
    memset(&SendMulticastAddr,0,sizeof(struct sockaddr_in));
    SendMulticastAddr.sin_family = AF_INET;
    SendMulticastAddr.sin_addr.s_addr = inet_addr(MCAST_SEND_ADDR);
    SendMulticastAddr.sin_port = htons(MCAST_SEND_PORT);

    /* 绑定自己的端口和IP信息到socket上 */
    if(bind(SendSocketFd,(struct sockaddr*)&SendMulticastAddr,sizeof(struct sockaddr_in)) < 0){
        perror("bind()");
        ::close(RecvSocketFd);
        ::close(SendSocketFd);
        return;
    }

    /* 绑定自己的端口和IP信息到socket上 */
    if(bind(RecvSocketFd,(struct sockaddr*)&RecvMulticastAddr,sizeof(struct sockaddr_in)) < 0){
        perror("bind()");
        ::close(RecvSocketFd);
        ::close(SendSocketFd);
        return;
    }

    /*禁止回环许可,即本机不能收到自己发送给组播地址的消息*/
    int loop = 0;
    if(setsockopt(SendSocketFd,IPPROTO_IP,IP_MULTICAST_LOOP,(const char*)&loop,sizeof(loop)) < 0){
        perror("setsockopt():IP_MULTICAST_LOOP");
        ::close(RecvSocketFd);
        ::close(SendSocketFd);
        return;
    }

    memset(&mreq,0,sizeof(mreq));
    mreq.imr_multiaddr.s_addr = inet_addr(MCAST_RECV_ADDR);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if(setsockopt(RecvSocketFd,IPPROTO_IP,IP_ADD_MEMBERSHIP,&mreq,
                  sizeof(mreq)) < 0){
        perror("setsockopt():IP_ADD_MEMBERSHIP");
        ::close(RecvSocketFd);
        ::close(SendSocketFd);
        return;
    }

    iSocketLen = sizeof(struct sockaddr_in);
#endif

#ifdef Q_OS_WIN32
    WSADATA wsaData;
    if(WSAStartup(MAKEWORD(2,2),&wsaData) != NO_ERROR){
        qDebug() << "初始化winsocket失败";
        return;
    }

    RecvSocketFd = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED);
    if(RecvSocketFd == INVALID_SOCKET){
        WSACleanup();
        qDebug() << "RecvSocketFd" << WSAGetLastError();
        return;
    }

    SendSocketFd = WSASocket(AF_INET,SOCK_DGRAM,0,NULL,0,WSA_FLAG_MULTIPOINT_C_LEAF | WSA_FLAG_MULTIPOINT_D_LEAF | WSA_FLAG_OVERLAPPED);
    if(SendSocketFd == INVALID_SOCKET){
        WSACleanup();
        closesocket(RecvSocketFd);
        qDebug() << "SendSocketFd" << WSAGetLastError();
        return;
    }

    //设置非阻塞模式
    u_long iMode = 1;
    if(ioctlsocket(RecvSocketFd,FIONBIO,&iMode) != NO_ERROR){
        WSACleanup();
        closesocket(RecvSocketFd);
        closesocket(SendSocketFd);
        qDebug() << "ioctlsocket" << WSAGetLastError();
        return;
    }

    //用来与RecvSocketFd进行绑定
    memset(&RecvMulticastAddr,0,sizeof(struct sockaddr_in));
    RecvMulticastAddr.sin_family = AF_INET;
    RecvMulticastAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    RecvMulticastAddr.sin_port = htons(MCAST_RECV_PORT);

    //用来与SendSocketFd进行绑定
    memset(&SendMulticastAddr,0,sizeof(struct sockaddr_in));
    SendMulticastAddr.sin_family = AF_INET;
    SendMulticastAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    SendMulticastAddr.sin_port = htons(MCAST_SEND_PORT);

    //用来从组播组接收消息
    memset(&RecvRemoteMulticastAddr,0,sizeof(struct sockaddr_in));
    RecvRemoteMulticastAddr.sin_family = AF_INET;
    RecvRemoteMulticastAddr.sin_addr.s_addr = inet_addr(MCAST_RECV_ADDR);
    RecvRemoteMulticastAddr.sin_port = htons(MCAST_RECV_PORT);

    //用来给组播组发送消息
    memset(&SendRemoteMulticastAddr,0,sizeof(struct sockaddr_in));
    SendRemoteMulticastAddr.sin_family = AF_INET;
    SendRemoteMulticastAddr.sin_addr.s_addr = inet_addr(MCAST_SEND_ADDR);
    SendRemoteMulticastAddr.sin_port = htons(MCAST_SEND_PORT);

    //绑定自己的端口和IP信息到socket上
    if(bind(RecvSocketFd,(struct sockaddr*)&RecvMulticastAddr,sizeof(struct sockaddr_in)) < 0){
        WSACleanup();
        closesocket(RecvSocketFd);
        closesocket(SendSocketFd);
        qDebug() << "bind RecvSocketFd" << WSAGetLastError();
        return;
    }

    //绑定自己的端口和IP信息到socket上
    if(bind(SendSocketFd,(struct sockaddr*)&SendMulticastAddr,sizeof(struct sockaddr_in)) < 0){
        WSACleanup();
        closesocket(RecvSocketFd);
        closesocket(SendSocketFd);
        qDebug() << "bind SendSocketFd" << WSAGetLastError();
        return;
    }

    //禁止回环许可
    const char loop = 0;
    if(setsockopt(SendSocketFd,IPPROTO_IP,IP_MULTICAST_LOOP,&loop,sizeof(loop)) < 0){
        WSACleanup();
        closesocket(RecvSocketFd);
        closesocket(SendSocketFd);
        qDebug() << "setsockopt SendSocketFd" << WSAGetLastError();
        return;
    }

    //加入组播
    if(WSAJoinLeaf(RecvSocketFd,(struct sockaddr*)&RecvRemoteMulticastAddr,
                   sizeof(struct sockaddr_in),
                   NULL,NULL,NULL,NULL,JL_RECEIVER_ONLY) == INVALID_SOCKET){
        WSACleanup();
        closesocket(RecvSocketFd);
        closesocket(SendSocketFd);
        qDebug() << "WSAJoinLeaf RecvSocketFd" << WSAGetLastError();
        return;
     }

    iSocketLen = sizeof(struct sockaddr_in);
#endif

}

UdpMulticastServer::~UdpMulticastServer()
{
    delete ui;
#ifdef ShuaKaJiConfigBranch
    delete shuakajiconfig;
#endif

#ifdef JiaYouZhanConfigBranch
    delete jiayouzhanconfig;
#endif
    delete device_upgrade;


#ifdef Q_OS_LINUX
    /*退出组播组*/
    setsockopt(RecvSocketFd,IPPROTO_IP,IP_DROP_MEMBERSHIP,&mreq,sizeof(mreq));
    ::close(RecvSocketFd);
    ::close(SendSocketFd);
#endif

#ifdef Q_OS_WIN32
    /*添加退出组播组*/
    closesocket(RecvSocketFd);
    closesocket(SendSocketFd);
    WSACleanup();
#endif
}

void UdpMulticastServer::on_btnSearchDevice_clicked()
{
    if(ui->btnSearchDevice->text() == tr("搜索")){
#ifdef Q_OS_LINUX
        if(sendto(SendSocketFd,MCAST_SEARCH_DEVICE,sizeof(MCAST_SEARCH_DEVICE),0,(struct sockaddr*)&SendMulticastAddr,sizeof(struct sockaddr_in)) < 0){
            perror("sendto()");
            return ;
        }
#endif

#ifdef Q_OS_WIN32
        if(sendto(SendSocketFd,MCAST_SEARCH_DEVICE,sizeof(MCAST_SEARCH_DEVICE),0,(struct sockaddr*)&SendRemoteMulticastAddr,sizeof(struct sockaddr_in)) < 0){
            qDebug() << "sendto" << WSAGetLastError();
            return;
        }
#endif

        ui->btnSearchDevice->setText(tr("停止"));
        ui->tableWidget->setSortingEnabled(false);
        ui->tableWidget->clear();
        tableWidgetSetting();
        row_index = 0;
        SelectedRowIndex = -1;
#ifdef ShuaKaJiConfigBranch
        query.exec(tr("DELETE FROM [dtm_shuakaji_device_configure_info]"));
#endif

#ifdef JiaYouZhanConfigBranch
        query.exec(tr("DELETE FROM [dtm_jiayouzhan_device_configure_info]"));
#endif
        ProcessDatagramTimer->start();
    }else if(ui->btnSearchDevice->text() == tr("停止")){
        ProcessDatagramTimer->stop();
        ui->btnSearchDevice->setText(tr("搜索"));
        ui->tableWidget->setSortingEnabled(true);
    }
}

void UdpMulticastServer::on_btnConfigureDevice_clicked()
{
    if(SelectedRowIndex == -1){
        QMessageBox::critical(this,tr("错误"),tr("请选择要配置的设备"));
        return;
    }

    QString SelectedDeviceIP =
            ui->tableWidget->item(SelectedRowIndex,0)->text();

#ifdef ShuaKaJiConfigBranch
    shuakajiconfig->LoadDefaultConfigure(SelectedDeviceIP);
    CommonSetting::WidgetCenterShow(*shuakajiconfig);
    shuakajiconfig->show();
#endif

#ifdef JiaYouZhanConfigBranch
    jiayouzhanconfig->LoadDefaultConfigure(SelectedDeviceIP);
    CommonSetting::WidgetCenterShow(*jiayouzhanconfig);
    jiayouzhanconfig->show();
#endif
}

void UdpMulticastServer::on_btnUpgradeDevice_clicked()
{
    if(SelectedRowIndex == -1){
        QMessageBox::critical(this,tr("错误"),tr("请选择要升级的设备"));
        return;
    }

    QString SelectedDeviceIP =
            ui->tableWidget->item(SelectedRowIndex,0)->text();
    device_upgrade->ServerIP = SelectedDeviceIP;
    qDebug() << SelectedDeviceIP;
    CommonSetting::WidgetCenterShow(*device_upgrade);
    device_upgrade->show();
}

void UdpMulticastServer::slotProcessPendingDatagrams()
{
    memset(ReceiveBuffer,0,BUFF_SIZE);
#ifdef Q_OS_LINUX
    if(recvfrom(RecvSocketFd,ReceiveBuffer,sizeof(ReceiveBuffer),0,(struct sockaddr*)&RecvMulticastAddr,&iSocketLen) < 0){
//        perror("recvfrom()");
        return;
    }
#endif

#ifdef Q_OS_WIN32
    if(recvfrom(RecvSocketFd,ReceiveBuffer,sizeof(ReceiveBuffer),0,(struct sockaddr*)&RecvRemoteMulticastAddr,&iSocketLen) < 0){
        qDebug() << "recvfrom" << WSAGetLastError();
        return;
    }
#endif

    qDebug() << ReceiveBuffer;

    QString ClientFeedBackInfo = QString(ReceiveBuffer);
    if((ClientFeedBackInfo.split("###").at(0) == "Client") &&
            (ClientFeedBackInfo.split("###").at(1) == "SearchDevice")){
#ifdef ShuaKaJiConfigBranch
        QString ClientNetWorkInfo =
                ClientFeedBackInfo.split("###").at(2);
        QStringList ClientNetWorkInfoList =
                ClientNetWorkInfo.split("\n");
        QString IP = ClientNetWorkInfoList.at(0).split("=").at(1);
        QString NetMask = ClientNetWorkInfoList.at(1).split("=").at(1);
        QString Gateway = ClientNetWorkInfoList.at(2).split("=").at(1);
        QString DNS = ClientNetWorkInfoList.at(3).split("=").at(1);
        QString MAC = ClientNetWorkInfoList.at(5).split("=").at(1);

        QString ClientConfigureInfo =
                ClientFeedBackInfo.split("###").at(3);
        CommonSetting::WriteFile("config.ini",ClientConfigureInfo);

        QString DeviceID =
                CommonSetting::ReadSettings("config.ini","DeviceID/ID");
        QString ServerIP =
                CommonSetting::ReadSettings("config.ini","ServerNetwork/IP");
        QString ServerListenPort =
                CommonSetting::ReadSettings("config.ini","ServerNetwork/PORT");
        QString HeartIntervalTime =
                CommonSetting::ReadSettings("config.ini","time/HeartIntervalTime");
        QString MaxTime =
                CommonSetting::ReadSettings("config.ini","time/MaxTime");
        QString RelayOnTime =
                CommonSetting::ReadSettings("config.ini","time/RelayOnTime");
        QString SmartUSBNumber =
                CommonSetting::ReadSettings("config.ini","SmartUSB/Num");

        if(ClientFeedBackInfo.split("###").size() == 5){
            QString VersionInfo = ClientFeedBackInfo.split("###").at(4);
            CommonSetting::WriteFile("VersionInfo.ini",VersionInfo);
        }
        QString Version =
                CommonSetting::ReadSettings("VersionInfo.ini","Version/VersionInfo");

        query.exec(tr("INSERT INTO [dtm_shuakaji_device_configure_info] ([develop_board_ip],[develop_board_mask],[develop_board_gateway],[develop_board_dns],[develop_board_mac],[device_id],[server_ip],[server_listen_port],[heart_interval_time],[max_time],[relay_on_time],[smartusb_number]) VALUES('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12')").arg(IP).arg(NetMask).arg(Gateway).arg(DNS).arg(MAC).arg(DeviceID).arg(ServerIP).arg(ServerListenPort).arg(HeartIntervalTime).arg(MaxTime).arg(RelayOnTime).arg(SmartUSBNumber));
        qDebug() << query.lastError();
#endif

#ifdef JiaYouZhanConfigBranch
        QString ClientNetWorkInfo =
                ClientFeedBackInfo.split("###").at(2);
        QStringList ClientNetWorkInfoList =
                ClientNetWorkInfo.split("\n");
        QString IP = ClientNetWorkInfoList.at(0).split("=").at(1);
        QString NetMask = ClientNetWorkInfoList.at(1).split("=").at(1);
        QString Gateway = ClientNetWorkInfoList.at(2).split("=").at(1);
        QString DNS = ClientNetWorkInfoList.at(3).split("=").at(1);
        QString MAC = ClientNetWorkInfoList.at(5).split("=").at(1);

        QString ClientConfigureInfo =
                ClientFeedBackInfo.split("###").at(3);
        CommonSetting::WriteFile("config.ini",ClientConfigureInfo);

        QString DeviceID =
                CommonSetting::ReadSettings("config.ini","DeviceID/ID");
        QString ServerIP =
                CommonSetting::ReadSettings("config.ini","ServerNetwork/IP");
        QString ServerListenPort =
                CommonSetting::ReadSettings("config.ini","ServerNetwork/PORT");
        QString HeartIntervalTime =
                CommonSetting::ReadSettings("config.ini","time/HeartIntervalTime");
        QString SwipCardIntervalTime =
                CommonSetting::ReadSettings("config.ini","time/SwipCardIntervalTime");

        if(ClientFeedBackInfo.split("###").size() == 5){
            QString VersionInfo = ClientFeedBackInfo.split("###").at(4);
            CommonSetting::WriteFile("VersionInfo.ini",VersionInfo);
        }
        QString Version =
                CommonSetting::ReadSettings("VersionInfo.ini","Version/VersionInfo");

        query.exec(tr("INSERT INTO [dtm_jiayouzhan_device_configure_info] ([develop_board_ip],[develop_board_mask],[develop_board_gateway],[develop_board_dns],[develop_board_mac],[device_id],[server_ip],[server_listen_port],[heart_interval_time],[swip_card_interval_time]) VALUES('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10')").arg(IP).arg(NetMask).arg(Gateway).arg(DNS).arg(MAC).arg(DeviceID).arg(ServerIP).arg(ServerListenPort).arg(HeartIntervalTime).arg(SwipCardIntervalTime));
        qDebug() << query.lastError();
#endif

#ifdef FaKaJiConfigBranch
        QString ClientNetWorkInfo =
                ClientFeedBackInfo.split("###").at(2);
        QStringList ClientNetWorkInfoList =
                ClientNetWorkInfo.split("\n");
        QString IP = ClientNetWorkInfoList.at(0).split("=").at(1);
        QString NetMask = ClientNetWorkInfoList.at(1).split("=").at(1);
        QString Gateway = ClientNetWorkInfoList.at(2).split("=").at(1);
        QString DNS = ClientNetWorkInfoList.at(3).split("=").at(1);
        QString MAC = ClientNetWorkInfoList.at(5).split("=").at(1);

        if(ClientFeedBackInfo.split("###").size() == 5){
            QString VersionInfo = ClientFeedBackInfo.split("###").at(4);
            CommonSetting::WriteFile("VersionInfo.ini",VersionInfo);
        }
        QString Version =
                CommonSetting::ReadSettings("VersionInfo.ini","Version/VersionInfo");
#endif
        QStringList list;
        list << IP << NetMask << Gateway << DNS << MAC << Version;
        for(int i = 0; i <list.count() ; i++){
            QTableWidgetItem *item = new QTableWidgetItem(list[i]);
            ui->tableWidget->setItem(row_index,i,item);
        }
        row_index++;
        tableSetAlignment();
    }else if((ClientFeedBackInfo.split("###").at(0) == "Client") &&
             (ClientFeedBackInfo.split("###").at(1) == "Configure")){
        QString ConfigureStatus = ClientFeedBackInfo.split("###").at(2);
        if(ConfigureStatus == tr("OK")){
            QMessageBox::information(this,tr("提示"),tr("配置成功,设备自动重启"));
#ifdef ShuaKaJiConfigBranch
            shuakajiconfig->close();
#endif

#ifdef JiaYouZhanConfigBranch
            jiayouzhanconfig->close();
#endif
            ProcessDatagramTimer->stop();
        }
    }
}

void UdpMulticastServer::slotSendConfigureInfo(QString ConfigureInfo)
{
    QString MessageHeader = QString("Server###Configure###");
    QString ConfigureDeviceIP = ui->tableWidget->item(SelectedRowIndex,0)->text();
    QString TotalConfigureMessage = MessageHeader + ConfigureDeviceIP + QString("###") + ConfigureInfo;

#ifdef Q_OS_LINUX
        if(sendto(SendSocketFd,TotalConfigureMessage.toAscii().data(),TotalConfigureMessage.count(),0,(struct sockaddr*)&SendMulticastAddr,sizeof(struct sockaddr_in)) < 0){
            perror("sendto()");
            return ;
        }
#endif

#ifdef Q_OS_WIN32
        if(sendto(SendSocketFd,TotalConfigureMessage.toAscii().data(),TotalConfigureMessage.count(),0,(struct sockaddr*)&SendRemoteMulticastAddr,sizeof(struct sockaddr_in)) < 0){
            qDebug() << "sendto" << WSAGetLastError();
            return;
        }
#endif

    ProcessDatagramTimer->stop();
    ProcessDatagramTimer->start();
}

void UdpMulticastServer::tableWidgetSetting()
{
    ui->tableWidget->setRowCount(100);
    ui->tableWidget->setColumnCount(6);
    ui->tableWidget->setHorizontalHeaderLabels(
                QStringList() << "IP" << "子网掩码" << "网关" << "DNS" << "MAC" << "版本");

    //设置单元格编辑模式
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //设置选择模式
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    //将行和列的大小设为与内容相匹配
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Stretch);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    //表格表头的显示与隐藏
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->verticalHeader()->setVisible(true);

    //所有单元格设置字体和字体大小
//    ui->tableWidget->setFont(QFont(tr("宋体"),16));

    //设置水平表头所有列的对齐方式和字体
    for(int i = 0; i < ui->tableWidget->columnCount(); i++){
        QTableWidgetItem *horizontalHeaderItem =
                ui->tableWidget->horizontalHeaderItem(i);
        horizontalHeaderItem->setTextAlignment(
                    Qt::AlignVCenter | Qt::AlignHCenter);
//        horizontalHeaderItem->setFont(QFont(tr("宋体"),16));
    }
}

void UdpMulticastServer::tableSetAlignment()
{
    //设置所有单元格的对齐方式和字体颜色
    for(int i = 0;i < ui->tableWidget->rowCount();i++){
        for(int j = 0;j < ui->tableWidget->columnCount();j++){
            QTableWidgetItem *CellItem = ui->tableWidget->item(i,j);
            if(CellItem){
                CellItem->setTextAlignment(
                            Qt::AlignVCenter | Qt::AlignHCenter);
            }
        }
    }
}

void UdpMulticastServer::on_tableWidget_cellClicked(int row, int column)
{
    if(ui->tableWidget->item(row,column) != NULL){
        SelectedRowIndex = row;
    }
}

void UdpMulticastServer::on_tableWidget_cellDoubleClicked(int row, int column)
{
#if defined(ShuaKaJiConfigBranch) || defined(JiaYouZhanConfigBranch)
    if(ui->tableWidget->item(row,column) != NULL){
        SelectedRowIndex = row;
        QString SelectedDeviceIP =
                ui->tableWidget->item(SelectedRowIndex,0)->text();

#ifdef ShuaKaJiConfigBranch
        shuakajiconfig->LoadDefaultConfigure(SelectedDeviceIP);
        CommonSetting::WidgetCenterShow(*shuakajiconfig);
        shuakajiconfig->show();
#endif

#ifdef JiaYouZhanConfigBranch
        jiayouzhanconfig->LoadDefaultConfigure(SelectedDeviceIP);
        CommonSetting::WidgetCenterShow(*jiayouzhanconfig);
        jiayouzhanconfig->show();
#endif
    }
#endif
}
