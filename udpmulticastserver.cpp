#include "udpmulticastserver.h"
#include "ui_udpmulticastserver.h"

//如果仅仅是想向一个组播组发送数据，而不要接受数据，那么可不用加入组播组，而直接通过sendto向组播组发送数据

UdpMulticastServer::UdpMulticastServer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::UdpMulticastServer)
{
    ui->setupUi(this);

    SelectedRowIndex = -1;
    tableWidgetSetting();

    shuakajiconfig = new ShuaKaJiConfig;
    connect(shuakajiconfig,SIGNAL(signalSendConfigureInfo(QString)),this,SLOT(slotSendConfigureInfo(QString)));

    jiayouzhanconfig = new JiaYouZhanConfig;
    connect(jiayouzhanconfig,SIGNAL(signalSendConfigureInfo(QString)),this,SLOT(slotSendConfigureInfo(QString)));

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

    delete shuakajiconfig;
    delete jiayouzhanconfig;

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
        query.exec(tr("DELETE FROM [dtm_device_configure_info]"));

#ifdef Q_OS_LINUX
        if(sendto(SendSocketFd,MCAST_SEARCH_DEVICE,sizeof(MCAST_SEARCH_DEVICE),0,(struct sockaddr*)&SendMulticastAddr,sizeof(struct sockaddr_in)) < 0){
            perror("sendto()");
            return ;
        }
#endif

#ifdef Q_OS_WIN32
        if(sendto(SendSocketFd,MCAST_SEARCH_DEVICE,sizeof(MCAST_SEARCH_DEVICE),0,(struct sockaddr*)&SendRemoteMulticastAddr,sizeof(struct sockaddr_in)) < 0)
        {
            qDebug() << "sendto" << WSAGetLastError();
            return;
        }
#endif

        ui->btnSearchDevice->setText(tr("停止"));
        ui->tableWidget->setSortingEnabled(false);
        ui->tableWidget->clearContents();
        SelectedRowIndex = -1;
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

    QString SelectedDeviceMac = ui->tableWidget->item(SelectedRowIndex,4)->text();

    if(ui->tableWidget->item(SelectedRowIndex,6)->text() == "ShuaKaJiConfigBranch"){
        shuakajiconfig->LoadDefaultConfigure(SelectedDeviceMac);
        CommonSetting::WidgetCenterShow(*shuakajiconfig);
        shuakajiconfig->show();
    }else if(ui->tableWidget->item(SelectedRowIndex,6)->text() == "JiaYouZhanConfigWiegBranch"){
        shuakajiconfig->LoadDefaultConfigure(SelectedDeviceMac);
        CommonSetting::WidgetCenterShow(*shuakajiconfig);
        shuakajiconfig->show();
    }else if(ui->tableWidget->item(SelectedRowIndex,6)->text() == "JiaYouZhanConfigCertificateBranch"){
        jiayouzhanconfig->LoadDefaultConfigure(SelectedDeviceMac);
        CommonSetting::WidgetCenterShow(*jiayouzhanconfig);
        jiayouzhanconfig->show();
    }else{
        QMessageBox::critical(this,tr("错误"),tr("本设备程序版本过低,请先升级"));
    }
}

void UdpMulticastServer::on_btnUpgradeDevice_clicked()
{
    if(SelectedRowIndex == -1){
        QMessageBox::critical(this,tr("错误"),tr("请选择要升级的设备"));
        return;
    }

    QString SelectedDeviceIP = ui->tableWidget->item(SelectedRowIndex,0)->text();
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
        QString ClientNetWorkInfo = ClientFeedBackInfo.split("###").at(2);
        QStringList ClientNetWorkInfoList = ClientNetWorkInfo.split("\n");
        QString IP = ClientNetWorkInfoList.at(0).split("=").at(1);
        QString NetMask = ClientNetWorkInfoList.at(1).split("=").at(1);
        QString Gateway = ClientNetWorkInfoList.at(2).split("=").at(1);
        QString DNS = ClientNetWorkInfoList.at(3).split("=").at(1);
        QString MAC = ClientNetWorkInfoList.at(5).split("=").at(1);

        QString ClientConfigureInfo = ClientFeedBackInfo.split("###").at(3);
        CommonSetting::WriteFile("config.ini",ClientConfigureInfo);
        QString ServerIP = CommonSetting::ReadSettings("config.ini","ServerNetwork/IP");
        QString ServerListenPort = CommonSetting::ReadSettings("config.ini","ServerNetwork/PORT");
        QString HeartIntervalTime = CommonSetting::ReadSettings("config.ini","time/HeartIntervalTime");
        QString SwipCardIntervalTime = CommonSetting::ReadSettings("config.ini","time/SwipCardIntervalTime");
        QString RelayOnTime = CommonSetting::ReadSettings("config.ini","time/RelayOnTime");
        QString MaxTime = CommonSetting::ReadSettings("config.ini","time/MaxTime");
        QString DeviceID = CommonSetting::ReadSettings("config.ini","DeviceID/ID");
        QString SmartUSBNumber = CommonSetting::ReadSettings("config.ini","SmartUSB/Num");

        QString Version;
        if(ClientFeedBackInfo.split("###").size() >= 5){
            QString VersionInfo = ClientFeedBackInfo.split("###").at(4);
            CommonSetting::WriteFile("VersionInfo.ini",VersionInfo);
            Version = CommonSetting::ReadSettings("VersionInfo.ini","Version/VersionInfo");
        }

        QString DeviceType;
        if(ClientFeedBackInfo.split("###").size() >= 6){
            DeviceType = ClientFeedBackInfo.split("###").at(5);
        }

        query.exec(tr("INSERT INTO [dtm_device_configure_info] ([server_ip],[server_listen_port],[heart_interval_time],[swip_card_interval_time],[relay_on_time],[max_time],[device_id],[develop_board_ip],[develop_board_gateway],[develop_board_mask],[develop_board_dns],[develop_board_mac],[smartusb_number],[version],[device_type]) VALUES('%1','%2','%3','%4','%5','%6','%7','%8','%9','%10','%11','%12','%13','%14','%15')").arg(ServerIP).arg(ServerListenPort).arg(HeartIntervalTime).arg(SwipCardIntervalTime).arg(RelayOnTime).arg(MaxTime).arg(DeviceID).arg(IP).arg(Gateway).arg(NetMask).arg(DNS).arg(MAC).arg(SmartUSBNumber).arg(Version).arg(DeviceType));
        qDebug() << query.lastError();

        ui->tableWidget->clearContents();
        int row_index = 0;
        query.exec(tr("SELECT [develop_board_ip],[develop_board_gateway],[develop_board_mask],[develop_board_dns],[develop_board_mac],[version],[device_type] FROM [dtm_device_configure_info]"));
        while(query.next()){
            for(int i = 0; i < 7; i++){
                QTableWidgetItem *item = new QTableWidgetItem(query.value(i).toString());
                ui->tableWidget->setItem(row_index,i,item);
            }
            row_index++;
        }
        tableSetAlignment();
    }else if((ClientFeedBackInfo.split("###").at(0) == "Client") &&
             (ClientFeedBackInfo.split("###").at(1) == "Configure")){
        QString ConfigureStatus = ClientFeedBackInfo.split("###").at(2);

        if(ConfigureStatus == tr("OK")){
            QMessageBox::information(this,tr("提示"),tr("配置成功,设备自动重启"));
            shuakajiconfig->close();
            jiayouzhanconfig->close();
            ProcessDatagramTimer->stop();
        }
    }
}

void UdpMulticastServer::slotSendConfigureInfo(QString ConfigureInfo)
{
    QString MessageHeader = QString("Server###Configure###");
    QString ConfigureDeviceMac = ui->tableWidget->item(SelectedRowIndex,4)->text();
    QString TotalConfigureMessage = MessageHeader + ConfigureDeviceMac + QString("###") + ConfigureInfo;

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
    ui->tableWidget->setColumnCount(7);
    ui->tableWidget->setHorizontalHeaderLabels(
                QStringList() << "IP" << "网关" << "子网掩码" << "DNS" << "MAC" << "版本" << "设备类型");

    //设置单元格编辑模式
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //设置选择模式
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);

    //将行和列的大小设为与内容相匹配
    ui->tableWidget->resizeRowsToContents();
    ui->tableWidget->resizeColumnsToContents();
//    ui->tableWidget->horizontalHeader()->setResizeMode(QHeaderView::Fixed);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);

    int DeskWidth = qApp->desktop()->availableGeometry().width() - 30;
    ui->tableWidget->setColumnWidth(0,DeskWidth * 0.125);
    ui->tableWidget->setColumnWidth(1,DeskWidth * 0.125);
    ui->tableWidget->setColumnWidth(2,DeskWidth * 0.125);
    ui->tableWidget->setColumnWidth(3,DeskWidth * 0.125);
    ui->tableWidget->setColumnWidth(4,DeskWidth * 0.15);
    ui->tableWidget->setColumnWidth(5,DeskWidth * 0.1);
    ui->tableWidget->setColumnWidth(6,DeskWidth * 0.15);

    //表格表头的显示与隐藏
    ui->tableWidget->horizontalHeader()->setVisible(true);
    ui->tableWidget->verticalHeader()->setVisible(true);

    //设置水平表头所有列的对齐方式和字体
    for(int i = 0; i < ui->tableWidget->columnCount(); i++){
        QTableWidgetItem *horizontalHeaderItem =
                ui->tableWidget->horizontalHeaderItem(i);
        horizontalHeaderItem->setTextAlignment(
                    Qt::AlignVCenter | Qt::AlignHCenter);
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
    if(ui->tableWidget->item(row,column) != NULL){
        SelectedRowIndex = row;
        QString SelectedDeviceMac = ui->tableWidget->item(SelectedRowIndex,4)->text();

        if(ui->tableWidget->item(SelectedRowIndex,6)->text() == "ShuaKaJiConfigBranch"){
            shuakajiconfig->LoadDefaultConfigure(SelectedDeviceMac);
            CommonSetting::WidgetCenterShow(*shuakajiconfig);
            shuakajiconfig->show();
        }else if(ui->tableWidget->item(SelectedRowIndex,6)->text() == "JiaYouZhanConfigWiegBranch"){
            shuakajiconfig->LoadDefaultConfigure(SelectedDeviceMac);
            CommonSetting::WidgetCenterShow(*shuakajiconfig);
            shuakajiconfig->show();
        }else if(ui->tableWidget->item(SelectedRowIndex,6)->text() == "JiaYouZhanConfigCertificateBranch"){
            jiayouzhanconfig->LoadDefaultConfigure(SelectedDeviceMac);
            CommonSetting::WidgetCenterShow(*jiayouzhanconfig);
            jiayouzhanconfig->show();
        }else{
            QMessageBox::critical(this,tr("错误"),tr("本设备程序版本过低,请先升级"));
        }
    }
}
