#ifndef COMMONSETTING_H
#define COMMONSETTING_H
#include <QObject>
#include <QBuffer>
#include <QComboBox>
#include <QListWidget>
#include <QDomDocument>
#include <QSettings>
#include <QWidget>
#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>
#include <QHostAddress>
#include <QLineEdit>
#include <QGridLayout>
#include <QStringList>
#include <QDesktopWidget>
#include <QDesktopServices>
#include <QSplashScreen>
#include <QList>
#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QTextCodec>
#include <QMessageBox>
#include <QAbstractButton>
#include <QPushButton>
#include <QTime>
#include <QDateTime>
#include <QDate>
#include <QCoreApplication>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QDir>
#include <QCursor>
#include <QTimer>
#include <QLabel>
#include <QApplication>
#include <QStyleFactory>
#include <QTextStream>
#include <QDebug>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QMouseEvent>
#include <QMutex>
#include <QMenu>
#include <QTranslator>
#include <stdio.h>
#include <stdlib.h>
#include <string>


class CommonSetting : public QObject
{
public:
    CommonSetting();
    ~CommonSetting();

    //设置编码为GB2312
    static void SetGB2312Code()
    {
        QTextCodec *codec =
                QTextCodec::codecForName("GB2312");
        QTextCodec::setCodecForLocale(codec);
        //        QTextCodec::setCodecForCStrings(codec);
        //        QTextCodec::setCodecForTr(codec);
    }

    //设置编码为UTF8
    static void SetUTF8Code()
    {
        QTextCodec *codec=
                QTextCodec::codecForName("UTF-8");
        QTextCodec::setCodecForLocale(codec);
        QTextCodec::setCodecForCStrings(codec);
        QTextCodec::setCodecForTr(codec);
    }

    //设置全局样式
    static void SetCustomStyle()
    {        
        QFile qssFile(":/qss/black.qss");
        if (qssFile.open(QFile::ReadOnly)) {
            QString qss = QLatin1String(qssFile.readAll());
            qApp->setStyleSheet(qss);
            QString PaletteColor = qss.mid(20, 7);
            qApp->setPalette(QPalette(QColor(PaletteColor)));
            qssFile.close();
        }
    }

    static void InstallChineseTranslator()
    {
        //加载Qt中的资源文件，使Qt显示中文（包括QMessageBox、文本框右键菜单等）
        QTranslator translator;
        translator.load(":/qm/qt_zh_CN.qm");
        qApp->installTranslator(&translator);
    }

    static void OpenDataBase()
    {
#ifdef __i386__
        QString dbFile = CommonSetting::GetCurrentPath() + QString("../database/DataBase.db");
//        QString dbFile = CommonSetting::GetCurrentPath() + QString("database/DataBase.db");
#endif

#ifdef __arm__
        QString dbFile = CommonSetting::GetCurrentPath() + QString("database/DataBase.db");
#endif
        if(!CommonSetting::FileExists(dbFile)){
            CommonSetting::QMessageBoxOnlyOk_Error(QString("%1数据库文件不存在！程序将自动关闭！").arg(dbFile));
            exit(-1);
        }
        QSqlDatabase DbConn =
                QSqlDatabase::addDatabase("QSQLITE");
        DbConn.setDatabaseName(dbFile);

        if(!DbConn.open()){
            CommonSetting::QMessageBoxOnlyOk_Error("打开数据库失败！程序将自动关闭！");
            exit(-1);
        }
    }

    //延时处理
    static void DelayMs(int msc)
    {
        QTime t = QTime::currentTime().addMSecs(msc);
        while(QTime::currentTime() < t)
        {
            QCoreApplication::processEvents(QEventLoop::AllEvents,100);
        }
    }

    //获取当前日期时间星期
    static QString GetCurrentDateTime()
    {
        //        QDateTime time = QDateTime::currentDateTime();
        //        return time.toString("yyyy-MM-dd hh:mm:ss");
        QString date = QDate::currentDate().toString("yyyy-MM-dd");
        QString time = QTime::currentTime().toString("hh:mm:ss");
        return date + "-" +  time;
    }

    //读取文件内容
    static QString ReadFile(const QString &fileName)
    {
        QFile file(fileName);
        QByteArray fileContent;
        if (!file.open(QIODevice::ReadOnly)){
            fileContent = "";
            QMessageBoxOnlyOk_Error(fileName + ":" +
                                    file.errorString());
        }else{
            fileContent = file.readAll();
        }
        file.close();
        return QString(fileContent);
    }

    //写数据到文件
    static void WriteFile(QString fileName,const QString data)
    {
        QFile file(fileName);
        if(!file.open(QFile::WriteOnly |
                      QFile::Truncate)){
            QMessageBoxOnlyOk_Error(fileName + ":"
                                    + file.errorString());
        }else{
            file.write(data.toLocal8Bit().data());
            file.close();
        }
    }

    //创建文件夹
    static void CreateFolder(QString path,QString strFolder)
    {
        QDir dir(path);
        dir.mkdir(strFolder);
    }

    //删除文件夹
    static bool deleteDir(const QString &dirName)
    {
        QDir directory(dirName);
        if (!directory.exists()){
            return true;
        }

        QString srcPath =
                QDir::toNativeSeparators(dirName);
        if (!srcPath.endsWith(QDir::separator()))
            srcPath += QDir::separator();

        QStringList fileNames =
                directory.entryList(QDir::AllEntries |
                                    QDir::NoDotAndDotDot | QDir::Hidden);
        bool error = false;
        for (QStringList::size_type i=0; i != fileNames.size(); ++i){
            QString filePath = srcPath +
                    fileNames.at(i);
            QFileInfo fileInfo(filePath);
            if(fileInfo.isFile() ||
                    fileInfo.isSymLink()){
                QFile::setPermissions(filePath, QFile::WriteOwner);
                if (!QFile::remove(filePath)){
                    qDebug() << "remove file" << filePath << " faild!";
                    error = true;
                }
            }else if (fileInfo.isDir()){
                if (!deleteDir(filePath)){
                    error = true;
                }
            }
        }

        if (!directory.rmdir(
                    QDir::toNativeSeparators(
                        directory.path()))){
            qDebug() << "remove dir" << directory.path() << " faild!";
            error = true;
        }

        return !error;
    }

    //拷贝文件：
    static bool copyFileToPath(QString sourceDir ,QString toDir, bool coverFileIfExist)
    {
        toDir.replace("\\","/");
        if (sourceDir == toDir){
            return true;
        }
        if (!QFile::exists(sourceDir)){
            return false;
        }
        QDir *createfile     = new QDir;
        bool exist = createfile->exists(toDir);
        if (exist){
            if(coverFileIfExist){
                createfile->remove(toDir);
            }
        }//end if

        if(!QFile::copy(sourceDir, toDir))
        {
            return false;
        }
        return true;
    }

    //拷贝文件夹
    static bool copyDirectoryFiles(const QString &fromDir, const QString &toDir, bool coverFileIfExist)
    {
        QDir sourceDir(fromDir);
        QDir targetDir(toDir);
        if(!targetDir.exists()){//如果目标目录不存在,则进行创建
            if(!targetDir.mkdir(
                        targetDir.absolutePath()))
                return false;
        }

        QFileInfoList fileInfoList =
                sourceDir.entryInfoList();
        foreach(QFileInfo fileInfo, fileInfoList){
            if(fileInfo.fileName() == "." ||
                    fileInfo.fileName() == "..")
                continue;

            if(fileInfo.isDir()){//当为目录时，递归的进行copy
                if(!copyDirectoryFiles(
                            fileInfo.filePath(),
                            targetDir.filePath(
                                fileInfo.fileName()),
                            coverFileIfExist))
                    return false;
            }else{//当允许覆盖操作时，将旧文件进行删除操作
                if(coverFileIfExist && targetDir.exists(
                            fileInfo.fileName())){
                    targetDir.remove(
                                fileInfo.fileName());
                }

                //进行文件copy
                if(!QFile::copy(fileInfo.filePath(),
                                targetDir.filePath(
                                    fileInfo.fileName()))){
                    return false;
                }
            }
        }
        return true;
    }

    //返回指定路径下符合筛选条件的文件，注意只返回文件夹名，不返回绝对路径
    static QStringList GetFileNames(QString path,QString filter)
    {
        QDir dir;
        dir.setPath(path);
        QStringList fileFormat(filter);
        dir.setNameFilters(fileFormat);
        dir.setFilter(QDir::Files);
        return dir.entryList();
    }
    //返回指定路径下文件夹的集合,注意只返回文件夹名，不返回绝对路径
    static QStringList GetDirNames(QString path)
    {
        QDir dir(path);
        dir.setFilter(QDir::Dirs);
        QStringList dirlist = dir.entryList();
        QStringList dirNames;
        foreach(const QString &dirName,dirlist){
            if(dirName == "." || dirName == "..")
                continue;
            dirNames << dirName;
        }
        return dirNames;
    }

    //返回文件文件时间
    static QString GetFileCreateTime(QString fileName)
    {
        QFileInfo file(fileName);
        QDateTime DateTime = file.created();
        return DateTime.toString("yyyy-MM-dd hh:mm:ss");
    }


    //将图片保存到QByteArray中
    static QByteArray SaveImageToQByteArrayInJPG(
            const QString &imageName)
    {
        QImage image(imageName);
        QByteArray tempData;
        QBuffer tempBuffer(&tempData);
        image.save(&tempBuffer,"JPG");
        return tempData;
    }

    //将JPG格式的图片数据保存到文件中
    static void SaveJPGDataToPic(const QByteArray &data,
                                 QString &picName)
    {
        QImage image;
        image.loadFromData(data);
        image.save(picName);
    }

    //QSetting应用
    static void WriteSettings(const QString &ConfigFile,
                              const QString &key,
                              const QString value)
    {

        QSettings settings(ConfigFile,QSettings::IniFormat);
        settings.setValue(key,value);
    }

    static QString ReadSettings(const QString &ConfigFile,
                                const QString &key)
    {
        QSettings settings(ConfigFile,QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        return settings.value(key).toString();
    }

    static QString ReadSettingsChinese(const QString &ConfigFile,
                                       const QString &key)
    {
        QSettings settings(ConfigFile,QSettings::IniFormat);
        settings.setIniCodec("UTF-8");
        return settings.value(key).toString();
    }

    static QStringList AllChildKeys(const QString &ConfigFile,const QString &beginGroup)
    {
        QSettings settings(ConfigFile,
                           QSettings::IniFormat);
        settings.beginGroup(beginGroup);
        return settings.childKeys();
    }

    static void RemoveSettingsKey(const QString &ConfigFile,
                                  const QString &key)
    {
        QSettings settings(ConfigFile,QSettings::IniFormat);
        settings.remove(key);
    }

    static void ClearSettings(const QString &ConfigFile)
    {
        QSettings settings(ConfigFile,QSettings::IniFormat);
        settings.clear();
    }

    //将指定路径下指定格式的文件返回(只返回文件名，不返回绝对路径)
    static QStringList fileFilter(const QString &path,
                                  const QString &filter)
    {
        QDir dir(path);
        QStringList fileFormat(filter);
        dir.setNameFilters(fileFormat);
        dir.setSorting(QDir::Time);
        dir.setFilter(QDir::Files);
        QStringList fileList = dir.entryList();
        return fileList;
    }


    //返回当前可执行程序的绝对路径,但不包括本身
    static QString GetExecutableProgramPath()
    {
        return QApplication::applicationDirPath();

    }

    //返回当前可执行程序的绝对路径,包括本身
    static QString GetExecutableProgramAbsolutePath()
    {
        return QApplication::applicationFilePath();
    }

    //获取当前路径
    static QString GetCurrentPath()
    {
        return QString(
                    QCoreApplication::applicationDirPath()+"/");
    }

    static void QMessageBoxOnlyOk_Information(const QString &info)
    {
        QMessageBox msg;
        msg.setWindowTitle(tr("提示"));
        msg.setText(info);
        msg.setIcon(QMessageBox::Information);
        msg.addButton(tr("确定"),QMessageBox::ActionRole);
        msg.exec();
    }

    static void QMessageBoxOnlyOk_Error(const QString &info)
    {
        QMessageBox msg;
        msg.setWindowTitle(tr("提示"));
        msg.setStyleSheet("font:18pt '宋体'");
        msg.setText(info);
        msg.setIcon(QMessageBox::Critical);
        msg.addButton(tr("确定"),QMessageBox::ActionRole);
        msg.exec();
    }

    static void QMessageBoxOkCancel(const QString &info)
    {
        QMessageBox msg;
        msg.setWindowTitle(tr("提示"));
        msg.setText(info);
        msg.setIcon(QMessageBox::Information);
        msg.addButton(tr("确定"),QMessageBox::ActionRole);
        msg.addButton(tr("取消"),QMessageBox::ActionRole);
        msg.exec();
    }

    //显示错误框，仅确定按钮
    static void ShowMessageBoxError(QString info)
    {
        QMessageBox msg;
        msg.setStyleSheet("font:12pt '宋体'");
        msg.setWindowTitle(tr("错误"));
        msg.setText(info);
        msg.setIcon(QMessageBox::Critical);
        msg.addButton(tr("确定"),QMessageBox::ActionRole);
        msg.exec();
    }

    //窗体居中显示
    static void WidgetCenterShow(QWidget &frm)
    {
        QDesktopWidget desktop;
        int screenX = desktop.availableGeometry().width();
        int screenY = desktop.availableGeometry().height();
        int wndX = frm.width();
        int wndY = frm.height();
        QPoint movePoint(screenX/2-wndX/2,screenY/2-wndY/2);
        frm.move(movePoint);
    }

    //窗体没有最大化按钮
    static void FormNoMaxButton(QWidget &frm)
    {
        frm.setWindowFlags(Qt::WindowMinimizeButtonHint);
    }

    //窗体没有最大化和最小化按钮
    static void FormOnlyCloseButton(QWidget &frm)
    {
        frm.setWindowFlags(Qt::WindowMinMaxButtonsHint);
        frm.setWindowFlags(Qt::WindowCloseButtonHint);
    }

    //窗体不能改变大小
    static void FormNotResize(QWidget  &frm)
    {
        frm.setFixedSize(frm.width(),frm.height());
    }

    //获取桌面大小
    static QSize GetDesktopSize()
    {
        QDesktopWidget desktop;
        return
                QSize(desktop.availableGeometry().width(),desktop.availableGeometry().height());
    }

    //文件是否存在
    static bool FileExists(QString strFile)
    {
        QFile tempFile(strFile);
        if (tempFile.exists())
        {
            return true;
        }
        return false;
    }

    static void SettingSystemDateTime(QString SystemDate)
    {
        //设置系统时间
        QString year = SystemDate.mid(0,4);
        QString month = SystemDate.mid(5,2);
        QString day = SystemDate.mid(8,2);
        QString hour = SystemDate.mid(11,2);
        QString minute = SystemDate.mid(14,2);
        QString second = SystemDate.mid(17,2);
        QProcess *process = new QProcess;
        process->start(tr("date %1%2%3%4%5.%6").arg(month).arg(day)
                       .arg(hour).arg(minute)
                       .arg(year).arg(second));
        process->waitForFinished(200);
        process->start("hwclock -w");
        process->waitForFinished(200);
        delete process;
    }

    static void Sleep(quint16 msec)
    {
        QTime dieTime = QTime::currentTime().addMSecs(msec);
        while(QTime::currentTime() < dieTime)
            QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }

    static QString AddHeaderByte(QString MessageMerge)
    {
        //添加头20个字节:类似ICARD:00009871000000
        QString TotalLength =
                QString::number(MessageMerge.length() + 20);
        int len = TotalLength.length();
        for (int i = 0; i < 8 - len; i++)
        {
            TotalLength = "0" + TotalLength;
        }

        QString temp = QString("%1%2%3%4").arg("ICARD:")
                .arg(TotalLength)
                .arg("000000")
                .arg(MessageMerge);
        return temp;
    }

#if 0
    //将打印信息输出到控制台
    static void WriteMsgConsole(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        static QMutex mutex;
        mutex.lock();

        QByteArray localMsg = msg.toLocal8Bit();
        switch (type) {
        case QtDebugMsg:
            fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtWarningMsg:
            fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtCriticalMsg:
            fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            break;
        case QtFatalMsg:
            fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), context.file, context.line, context.function);
            abort();
        }

        mutex.unlock();
    }

    //将打印信息都写到文件中
    static void WriteMsgFile(QtMsgType type, const QMessageLogContext &context, const QString &msg)
    {
        static QMutex mutex;
        mutex.lock();

        QString text;
        switch(type)
        {
        case QtDebugMsg:
            text = QString("Debug:");
            break;
        case QtWarningMsg:
            text = QString("Warning:");
            break;
        case QtCriticalMsg:
            text = QString("Critical:");
            break;
        case QtFatalMsg:
            text = QString("Fatal:");
        }

        QString context_info = QString("File:(%1) Line:(%2)").arg(context.file).arg(context.line);
        QString current_date_time =
                QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss ddd");
        QString message = QString("%1 %2 %3 %4").arg(text).arg(context_info).arg(msg).arg(current_date_time);

        QFile file("log.txt");
        file.open(QIODevice::WriteOnly |
                  QIODevice::Append);
        QTextStream text_stream(&file);
        text_stream << message << "\r\n";
        file.flush();
        file.close();

        mutex.unlock();
    }
#endif

};
#endif // COMMONSETTING_H
