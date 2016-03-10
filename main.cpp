#include "udpmulticastserver.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    CommonSetting::SetUTF8Code();
    CommonSetting::OpenDataBase();

    qApp->setFont(QFont("微软雅黑",14));

    UdpMulticastServer main_form;
    main_form.showMaximized();

    return a.exec();
}
