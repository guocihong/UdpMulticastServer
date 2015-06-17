#include "deviceupgrade.h"
#include "ui_deviceupgrade.h"

DeviceUpgrade::DeviceUpgrade(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DeviceUpgrade)
{
    ui->setupUi(this);

    send = new SendFileClient(this);
    connect(send, SIGNAL(fileSendSucceed()), this, SLOT(sendFinsh()));
    connect(send, SIGNAL(disconnected()), this, SLOT(slotCloseConnection()));
    connect(send, SIGNAL(fileSize(qint64)), this, SLOT(setSendPBar(qint64)));
    connect(send, SIGNAL(signalBytesWritten(qint64)), this, SLOT(updateSendPBar(qint64)));
}

DeviceUpgrade::~DeviceUpgrade()
{
    delete ui;
}

void DeviceUpgrade::on_btnOpen_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("选择升级文件"),".", tr("*;;*.tgz *.tar.bz2 *.tar.gz"));
    ui->lineEdit->setText(fileName);
    ui->progressBar->setValue(0);
    ui->UpgradeStatusLabel->clear();
}

void DeviceUpgrade::on_btnUpgrade_clicked()
{
    QString file = ui->lineEdit->text();
    if(file.isEmpty()){
        QMessageBox::critical(this,tr("错误"),tr("发送文件不能为空"));
        return;
    }

    ui->progressBar->setValue(0);
    ui->UpgradeStatusLabel->setText(tr("正在升级..."));
    ui->btnUpgrade->setDisabled(true);

#if defined(Q_OS_WIN32)
    system("netsh firewall set opmode mode=disable");
#endif

    fileSize = 0;
    sendBytes = 0;
    send->SendFile(file, ServerIP, 5902);
}

void DeviceUpgrade::setSendPBar(qint64 size)
{
    fileSize = size;
    ui->progressBar->setRange(0, fileSize);
}

void DeviceUpgrade::updateSendPBar(qint64 size)
{
    sendBytes += size;
    ui->progressBar->setValue(sendBytes);
    qApp->processEvents();
}


void DeviceUpgrade::sendFinsh()
{
    ui->progressBar->setValue(fileSize);
    ui->UpgradeStatusLabel->setText(tr("升级成功"));
    QMessageBox::information(0,tr("提示"),tr("升级成功"));

    this->close();
}

void DeviceUpgrade::slotCloseConnection()
{
    ui->btnUpgrade->setEnabled(true);
}
