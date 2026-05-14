#include "mainWindow.h"
#include <QtGlobal>
#include <QFont>
#include <QtWidgets/QApplication>
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
#include <QGuiApplication>
#endif
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
    logger->info(QStringLiteral("                           App start                            "));
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
#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
    QCoreApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif
#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
    QGuiApplication::setHighDpiScaleFactorRoundingPolicy(
        Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);
#endif

    QApplication a(argc, argv);

    // 统一默认 UI 字体（点阵随 DPI 缩放），减少不同分辨率/系统缩放下与控件显式字体的差异
    QFont uiFont = a.font();
#ifdef Q_OS_WIN
    uiFont.setFamily(QStringLiteral("Microsoft YaHei UI"));
#endif
    uiFont.setPointSize(10);
    uiFont.setStyleHint(QFont::SansSerif, QFont::PreferAntialias);
    a.setFont(uiFont);

    // 启用新的日子记录类
    QString sConfFilename = "./log4qt.conf";
    if (QFileInfo::exists(sConfFilename)) {
        Log4Qt::PropertyConfigurator::configure(sConfFilename);
    }
    else {
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

        Log4Qt::DailyFileAppender* dailiAppender = new Log4Qt::DailyFileAppender(layout, "logs/.log", "logs_yyyy-MM-dd");
        dailiAppender->setAppendFile(true);
        dailiAppender->activateOptions();
        logger->addAppender(dailiAppender);
    }

    // 必须在安装 AppMessageHandler 之前启用：否则 qInstallMessageHandler 返回 nullptr，
    // 自定义处理函数无法把 qInfo 等转发到 Log4Qt（常见于仅有 log4qt.conf 且未配置 log4j.handleQtMessages=true）。
    Log4Qt::LogManager::setHandleQtMessages(true);

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
