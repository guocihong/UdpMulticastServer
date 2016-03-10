#include "shuakajiconfig.h"
#include "ui_shuakajiconfig.h"

ShuaKaJiConfig::ShuaKaJiConfig(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ShuaKaJiConfig)
{
    ui->setupUi(this);
}

ShuaKaJiConfig::~ShuaKaJiConfig()
{
    delete ui;
}

void ShuaKaJiConfig::on_btnSend_clicked()
{
    QString ServerIP = ui->ServerIP->text();
    QString ServerListenPort = ui->ServerListenPort->text();
    QString HeartIntervalTime = ui->HeartIntervalTime->text();
    QString RelayOnTime = ui->RelayOnTime->text();
    QString MaxTime = ui->MaxTime->text();
    QString DeviceID = ui->DeviceID->text();
    QString DevelopBoardIP = ui->DevelopBoardIP->text();
    QString DevelopBoardGateWay = ui->DevelopBoardGateWay->text();
    QString DevelopBoardMask = ui->DevelopBoardMask->text();
    QString DevelopBoardDNS = ui->DevelopBoardDNS->text();
    QString SmartUSBNumber = ui->SmartUSBNumber->text();

    QStringList ConfigureInfoList;
    ConfigureInfoList << ServerIP << ServerListenPort << HeartIntervalTime << "0" << RelayOnTime << MaxTime << DeviceID << DevelopBoardIP << DevelopBoardGateWay << DevelopBoardMask << DevelopBoardDNS << SmartUSBNumber;

    emit signalSendConfigureInfo(ConfigureInfoList.join(","));
}

void ShuaKaJiConfig::LoadDefaultConfigure(QString SelectedDeviceMac)
{
    query.exec(tr("SELECT [server_ip],[server_listen_port],[heart_interval_time],[relay_on_time],[max_time],[device_id],[develop_board_ip],[develop_board_gateway],[develop_board_mask],[develop_board_dns],[smartusb_number] FROM [dtm_device_configure_info] WHERE [develop_board_mac] = '%1'").arg(SelectedDeviceMac));
    while(query.next()){
        ui->ServerIP->setText(query.value(0).toString());
        ui->ServerListenPort->setText(query.value(1).toString());
        ui->HeartIntervalTime->setText(query.value(2).toString());
        ui->RelayOnTime->setText(query.value(3).toString());
        ui->MaxTime->setText(query.value(4).toString());
        ui->DeviceID->setText(query.value(5).toString());
        ui->DevelopBoardIP->setText(query.value(6).toString());
        ui->DevelopBoardGateWay->setText(query.value(7).toString());
        ui->DevelopBoardMask->setText(query.value(8).toString());
        ui->DevelopBoardDNS->setText(query.value(9).toString());
        ui->SmartUSBNumber->setText(query.value(10).toString());
    }
}
