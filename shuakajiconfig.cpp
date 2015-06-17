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
    ConfigureInfoList << ServerIP << ServerListenPort << HeartIntervalTime << RelayOnTime << MaxTime << DeviceID << DevelopBoardIP << DevelopBoardMask << DevelopBoardGateWay << DevelopBoardDNS << SmartUSBNumber;

    emit signalSendConfigureInfo(ConfigureInfoList.join(","));
}

void ShuaKaJiConfig::LoadDefaultConfigure(QString SelectedDeviceIP)
{
    query.exec(tr("SELECT * FROM [dtm_shuakaji_device_configure_info] WHERE [develop_board_ip] = '%1'").arg(SelectedDeviceIP));
    while(query.next()){
        ui->DevelopBoardIP->setText(query.value(0).toString());
        ui->DevelopBoardMask->setText(query.value(1).toString());
        ui->DevelopBoardGateWay->setText(query.value(2).toString());
        ui->DevelopBoardDNS->setText(query.value(3).toString());
        ui->DeviceID->setText(query.value(5).toString());
        ui->ServerIP->setText(query.value(6).toString());
        ui->ServerListenPort->setText(query.value(7).toString());
        ui->HeartIntervalTime->setText(query.value(8).toString());
        ui->MaxTime->setText(query.value(9).toString());
        ui->RelayOnTime->setText(query.value(10).toString());
        ui->SmartUSBNumber->setText(query.value(11).toString());
    }
}
