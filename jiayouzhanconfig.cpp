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

    QStringList ConfigureInfoList;
    ConfigureInfoList << ServerIP << ServerListenPort << HeartIntervalTime << SwipCardIntervalTime << DeviceID << DevelopBoardIP << DevelopBoardMask << DevelopBoardGateWay << DevelopBoardDNS;

    emit signalSendConfigureInfo(ConfigureInfoList.join(","));
}

void JiaYouZhanConfig::LoadDefaultConfigure(QString SelectedDeviceIP)
{
    query.exec(tr("SELECT * FROM [dtm_jiayouzhan_device_configure_info] WHERE [develop_board_ip] = '%1'").arg(SelectedDeviceIP));
    while(query.next()){
        ui->DevelopBoardIP->setText(query.value(0).toString());
        ui->DevelopBoardMask->setText(query.value(1).toString());
        ui->DevelopBoardGateWay->setText(query.value(2).toString());
        ui->DevelopBoardDNS->setText(query.value(3).toString());
        ui->DeviceID->setText(query.value(5).toString());
        ui->ServerIP->setText(query.value(6).toString());
        ui->ServerListenPort->setText(query.value(7).toString());
        ui->HeartIntervalTime->setText(query.value(8).toString());
        ui->SwipCardIntervalTime->setText(query.value(9).toString());
    }
}
