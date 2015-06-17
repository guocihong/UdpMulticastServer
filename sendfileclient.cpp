#include "sendfileclient.h"

SendFileClient::SendFileClient(QObject *parent) : QTcpSocket(parent)
{
    connect(this, SIGNAL(connected()), this, SLOT(SendData()));
    connect(this, SIGNAL(error(QAbstractSocket::SocketError)),
           this,SLOT(displaySocketError(QAbstractSocket::SocketError)));
}

SendFileClient::~SendFileClient()
{

}

void SendFileClient::SendFile(QString fileName, QString serverIP, int serverPort)
{
    this->fileName = fileName;
    connectToHost(serverIP, serverPort);
}

void SendFileClient::SendData()
{
    QFile file(fileName);
    if(!file.open(QIODevice::ReadOnly)) {
        QMessageBox::critical(0,tr("错误"),tr("文件打开失败"));
        disconnectFromHost();
        return;
    } else {
        emit fileSize(file.size());
    }

    qint64 size;
    QByteArray block;
    QDataStream out(&block, QIODevice::WriteOnly);
    out.setVersion(QDataStream::Qt_4_7);
    QString name = myHelper::GetFileNameWithExtension(fileName);

    //写入开始符及文件名称
    block.clear();
    out.device()->seek(0);
    out << 0x01 << name.toUtf8();
    size = block.size();
    write((char *)&size, sizeof(qint64));
    write(block.data(), size);

    if(!waitForBytesWritten(-1)) {
        QMessageBox::critical(0,tr("错误"),QString("发送开始符数据发生错误:%1").arg(errorString()));
        disconnectFromHost();
        return;
    }

    //写入文件大小
    block.clear();
    out.device()->seek(0);
    out << 0x02 << QString::number(file.size()).toUtf8();
    size = block.size();
    write((char *)&size, sizeof(qint64));
    write(block.data(), size);

    if(!waitForBytesWritten(-1)) {
        QMessageBox::critical(0,tr("错误"),QString("发送文件大小数据发生错误:%1").arg(errorString()));
        disconnectFromHost();
        return;
    }

    //循环写入文件数据
    do {
        block.clear();
        out.device()->seek(0);
        out << 0x03 << file.read(0xFFFF);
        size = block.size();
        write((char *)&size, sizeof(qint64));
        write(block.data(), size);
        if(!waitForBytesWritten(-1)) {
            QMessageBox::critical(0,tr("错误"),tr("发送文件数据发生错误:%1").arg(errorString()));
            disconnectFromHost();
            return;
        }
        emit signalBytesWritten(size - 12);
    } while(!file.atEnd());

    //写入结束符及文件名称
    block.clear();
    out.device()->seek(0);
    out << 0x04 << name.toUtf8();
    size = block.size();
    write((char *)&size, sizeof(qint64));
    write(block.data(), size);

    if(!waitForBytesWritten(-1)) {
        QMessageBox::critical(0,tr("错误"),tr("发送结束符数据发生错误:%1").arg(errorString()));
        disconnectFromHost();
        return;
    }

    while(1){
        if(waitForDisconnected(100)){
            break;
        }
        qApp->processEvents();
    }
    emit fileSendSucceed();
}

void SendFileClient::displaySocketError(QAbstractSocket::SocketError )
{
//    QMessageBox::critical(0,tr("错误"),QString("发生错误:%1").arg(errorString()));
}
