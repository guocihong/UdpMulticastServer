#ifndef JIAYOUZHANCONFIG_H
#define JIAYOUZHANCONFIG_H

#include <QWidget>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

namespace Ui {
class JiaYouZhanConfig;
}

class JiaYouZhanConfig : public QWidget
{
    Q_OBJECT

public:
    explicit JiaYouZhanConfig(QWidget *parent = 0);
    ~JiaYouZhanConfig();
    void LoadDefaultConfigure(QString SelectedDeviceIP);

private slots:
    void on_btnSend_clicked();

signals:
    void signalSendConfigureInfo(QString ConfigureInfo);

private:
    Ui::JiaYouZhanConfig *ui;
    QSqlQuery query;
};

#endif // JIAYOUZHANCONFIG_H
