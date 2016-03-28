#include "jiayouzhanconfig.h"
#include "ui_jiayouzhanconfig.h"

JiaYouZhanConfig::JiaYouZhanConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::JiaYouZhanConfig)
{
    ui->setupUi(this);
}

JiaYouZhanConfig::~JiaYouZhanConfig()
{
    delete ui;
}

void JiaYouZhanConfig::on_btnSend_clicked()
{
    QString ServerIP = ui->ServerIP->text();
    QString ServerListenPort = ui->ServerListenPort->text();
    QString HeartIntervalTime = ui->HeartIntervalTime->text();
    QString SwipCardIntervalTime = ui->SwipCardIntervalTime->text();
    QString DeviceID = ui->DeviceID->text();
    QString DevelopBoardIP = ui->DevelopBoardIP->text();
    QString DevelopBoardGateWay = ui->DevelopBoardGateWay->text();
    QString DevelopBoardMask = ui->DevelopBoardMask->text();
    QString DevelopBoardDNS = ui->DevelopBoardDNS->text();
    QString DevelopBoardMAC = ui->DevelopBoardMAC->text();

    QStringList ConfigureInfoList;
    ConfigureInfoList << ServerIP << ServerListenPort << HeartIntervalTime << SwipCardIntervalTime << "0" << "0" << DeviceID << DevelopBoardIP << DevelopBoardGateWay << DevelopBoardMask << DevelopBoardDNS << DevelopBoardMAC << "0";

    emit signalSendConfigureInfo(ConfigureInfoList.join(","));
}

void JiaYouZhanConfig::LoadDefaultConfigure(QString SelectedDeviceMac)
{
    query.exec(tr("SELECT [server_ip],[server_listen_port],[heart_interval_time],[swip_card_interval_time],[device_id],[develop_board_ip],[develop_board_gateway],[develop_board_mask],[develop_board_dns],[develop_board_mac] FROM [dtm_device_configure_info] WHERE [develop_board_mac] = '%1'").arg(SelectedDeviceMac));
    while(query.next()){
        ui->ServerIP->setText(query.value(0).toString());
        ui->ServerListenPort->setText(query.value(1).toString());
        ui->HeartIntervalTime->setText(query.value(2).toString());
        ui->SwipCardIntervalTime->setText(query.value(3).toString());
        ui->DeviceID->setText(query.value(4).toString());
        ui->DevelopBoardIP->setText(query.value(5).toString());
        ui->DevelopBoardGateWay->setText(query.value(6).toString());
        ui->DevelopBoardMask->setText(query.value(7).toString());
        ui->DevelopBoardDNS->setText(query.value(8).toString());
        ui->DevelopBoardMAC->setText(query.value(9).toString());
    }
}
