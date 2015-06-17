#ifndef SENDFILECLIENT_H
#define SENDFILECLIENT_H

#include "myhelper.h"

class SendFileClient : public QTcpSocket
{
    Q_OBJECT
public:
    explicit SendFileClient(QObject *parent = 0);
    ~SendFileClient();

    void SendFile(QString fileName, QString serverIP, int serverPort);

signals:
    void fileSize(qint64 size);
    void message(QString msg);
    void signalBytesWritten(qint64 sendSize);
    void fileSendSucceed();

private slots:
    void SendData();
    void displaySocketError(QAbstractSocket::SocketError);

private:
    QString fileName;

};

#endif // SENDFILECLIENT_H
