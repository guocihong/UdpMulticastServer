#ifndef SHUAKAJICONFIG_H
#define SHUAKAJICONFIG_H

#include <QWidget>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

namespace Ui {
class ShuaKaJiConfig;
}

class ShuaKaJiConfig : public QWidget
{
    Q_OBJECT

public:
    explicit ShuaKaJiConfig(QWidget *parent = 0);
    ~ShuaKaJiConfig();
    void LoadDefaultConfigure(QString SelectedDeviceIP);

private slots:
    void on_btnSend_clicked();

signals:
    void signalSendConfigureInfo(QString ConfigureInfo);

private:
    Ui::ShuaKaJiConfig *ui;
    QSqlQuery query;
};

#endif // SHUAKAJICONFIG_H
