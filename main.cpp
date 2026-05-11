#include "mainWindow.h"
#include <QtWidgets/QApplication>
#include "version.h"

#include <log4qt/log4qt.h>
#include <log4qt/logger.h>
#include <log4qt/layout.h>
#include <log4qt/patternlayout.h>
#include <log4qt/consoleappender.h>
#include <log4qt/dailyfileappender.h>
#include <log4qt/logmanager.h>
#include <log4qt/propertyconfigurator.h>
#include <log4qt/loggerrepository.h>
#include <log4qt/fileappender.h>

mainWindow* mw = nullptr;
QMutex mutexMsg;
QtMessageHandler system_default_message_handler = NULL;// 用来保存系统默认的输出接口
void AppMessageHandler(QtMsgType type, const QMessageLogContext& context, const QString& msg)
{
    //Release 版本默认不包含context这些信息:文件名、函数名、行数，需要在.pro项目文件加入以下代码，加入后最好重新构建项目使之生效：
    //DEFINES += QT_MESSAGELOGCONTEXT

    //在.pro文件定义以下的宏，可以屏蔽相应的日志输出
    //DEFINES += QT_NO_WARNING_OUTPUT
    //DEFINES += QT_NO_DEBUG_OUTPUT
    //DEFINES += QT_NO_INFO_OUTPUT
    //文件名、函数名、行数
    // strMsg += QString("Function: %1  File: %2  Line: %3 ").arg(context.function).arg(context.file).arg(context.line);

    // 加锁
    QMutexLocker locker(&mutexMsg);
    // if (type == QtWarningMsg)
    //     return;

    if (mw && type != QtDebugMsg)
        emit mw->sigAppendMsg(msg, type);

    //这里必须调用，否则消息被拦截，log4qt无法捕获系统日志
    if (system_default_message_handler) {
        system_default_message_handler(type, context, msg);
    }
}

void logStartup()
{
    auto logger = Log4Qt::Logger::rootLogger();

    logger->info(QStringLiteral("################################################################"));
    logger->info(QStringLiteral("                           程序启动                               "));
    logger->info(QStringLiteral("################################################################"));
}

void shutdownRootLogger()
{
    auto logger = Log4Qt::Logger::rootLogger();
    logger->removeAllAppenders();
    logger->loggerRepository()->shutdown();
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 启用新的日子记录类
    QString sConfFilename = "./log4qt.conf";
    if (QFileInfo::exists(sConfFilename)) {
        Log4Qt::PropertyConfigurator::configure(sConfFilename);
    }
    else {
        Log4Qt::LogManager::setHandleQtMessages(true);
        Log4Qt::Logger* logger = Log4Qt::Logger::rootLogger();
        logger->setLevel(Log4Qt::Level::DEBUG_INT); //设置日志输出级别

        /****************PatternLayout配置日志的输出格式****************************/
        Log4Qt::PatternLayout* layout = new Log4Qt::PatternLayout();
        layout->setConversionPattern("%d{yyyy-MM-dd HH:mm:ss.zzz} [%p]: %m %n");
        layout->activateOptions();

        /***************************配置日志的输出位置***********/
        //输出到控制台
        Log4Qt::ConsoleAppender* appender = new Log4Qt::ConsoleAppender(layout, Log4Qt::ConsoleAppender::STDOUT_TARGET);
        appender->activateOptions();
        logger->addAppender(appender);

        //输出到文件(如果需要把离线处理单独保存日志文件，可以改这里)
        QStringList args = QCoreApplication::arguments();
        if (args.contains("-m") && args.contains("offline")) {
            Log4Qt::DailyFileAppender* dailiAppender = new Log4Qt::DailyFileAppender(layout, "logs/.log", "offline_yyyy-MM-dd");
            dailiAppender->setAppendFile(true);
            dailiAppender->activateOptions();
            logger->addAppender(dailiAppender);
        }
        else {
            Log4Qt::DailyFileAppender* dailiAppender = new Log4Qt::DailyFileAppender(layout, "logs/.log", "online_yyyy-MM-dd");
            dailiAppender->setAppendFile(true);
            dailiAppender->activateOptions();
            logger->addAppender(dailiAppender);
        }
    }

    // 确保logs目录存在
    QDir dir(QDir::currentPath() + "/logs");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    logStartup();

    mainWindow w;
    mw = &w;  // 在消息处理程序使用mw之前，必须先初始化它

    //安装日志，主要用户主界面刷新日志信息，日志写文件改为log4qt模块来实现了
    system_default_message_handler = qInstallMessageHandler(AppMessageHandler);

    //打印软件版本号
    qInfo().noquote() << QObject::tr("系统启动，软件版本号: %1").arg(APP_VERSION);
    w.show();

    //运行运行到这里，此时主窗体析构函数还没触发，所以shutdownRootLogger需要在主窗体销毁以后再做处理
    QObject::connect(&w, &QObject::destroyed, [] {
        shutdownRootLogger();
    });

    return a.exec();//直接return 0 就达不到事件循环并显示的效果，程序直接退出了，而exec就是在这里进行了循环一直循环处理着用户和系统的事件。
}
