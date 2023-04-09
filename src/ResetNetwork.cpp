#include "ResetNetwork.h"
#include "mainWindow.h"

#include <QElapsedTimer>

#pragma execution_character_set("utf-8") 

// 控制继电器的窗口
ResetNetwork::ResetNetwork(QWidget* parent)
    : QDialog(parent)
{
    ui.setupUi(this);
    // 读取配置文件：json文件
    QJsonObject jsonSetting = mainWindow::ReadSetting();
    tcpIp = jsonSetting["IP_Detector"].toString();
    tcpGateway = jsonSetting["Gateway_Detector"].toString();
    tcpPort = jsonSetting["Port_Detector"].toString();
    

    ui.changeSetting->setEnabled(false);// 控制器开关禁止状态
    ui.IP_ARM_Edit->setText(tcpIp);
    ui.Gateway_ARM_Edit->setText(tcpGateway);
    ui.Port_ARM_Edit->setText(tcpPort);
    ui.Port_ARM_Edit->setValidator(new QIntValidator(1, 9999, this));  // 端口号只能在[1,9999]范围内的整数输入
    timer = Q_NULLPTR;
    tcpSocket = Q_NULLPTR;//使用前先清空 
}

ResetNetwork::~ResetNetwork()
{
    //定时器关闭
    if (timer) {
        if (timer->isActive())//判断定时器是否在工作
            timer->stop();
        delete timer;
    }
    if (tcpSocket) delete tcpSocket;
}

// 连接网络按钮
void ResetNetwork::on_connectButton_clicked()
{
    if (ui.connectButton->text() == "连接")
    {
        ui.connectButton->setEnabled(false); // 连接按钮禁止使用，待系统响应网络
        ui.NetStatusLabel->setText("网络连接中。。。");

        if (tcpSocket) delete tcpSocket; //如果有指向其他空间直接删除
        tcpSocket = new QTcpSocket(this); //申请堆空间有TCP发送和接受操作

        // 点击连接按钮后，记录下当前IP、网关以及Port
        tcpIp = ui.IP_ARM_Edit->text();
        tcpGateway = ui.Gateway_ARM_Edit->text();
        tcpPort = ui.Port_ARM_Edit->text();

        QJsonObject jsonSetting = mainWindow::ReadSetting();
        jsonSetting["IP_Detector"] = tcpIp;
        jsonSetting["Gateway_Detector"] = tcpGateway;
        jsonSetting["Port_Detector"] = tcpPort;
        mainWindow::WriteSetting(jsonSetting);

        if (!ui.IP_ARM_Edit->isTextValid(tcpIp))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Warning");
            msgBox.setText("IP is in valid");
            msgBox.exec();
            return;
        }
        if (!ui.Gateway_ARM_Edit->isTextValid(tcpGateway))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Warning");
            msgBox.setText("Gateway is in valid");
            msgBox.exec();
            return;
        }

        tcpSocket->connectToHost(tcpIp, tcpPort.toInt());//连接主机
        connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(displayError(QAbstractSocket::SocketError)));//错误连接
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectUpdata())); //更新连接之后按钮的使能
        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMassage())); //读取接收的信息
    }
    else if (ui.connectButton->text() == "断开")
    {
        //定时器关闭
        if (timer) 
        {
            if (timer->isActive())//判断定时器是否在工作
                timer->stop();
            delete timer;
        }

        tcpSocket->abort();//abort函数用于使程序非正常中止/异常退出
        if (tcpSocket) delete tcpSocket;
        tcpSocket = NULL;
        disconnectUpdata();
    }
}

// 在点击连接后，错误连接：即无法连接网络/失去连接，则进入该函数
void ResetNetwork::displayError(QAbstractSocket::SocketError)
{
    QMessageBox::warning(this, tr("Warnning"), tcpSocket->errorString());
    tcpSocket->close();

    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(255,0,0);"//红色
        "border: 2px solid rgb(178, 34, 34);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(255, 165, 0);"
        "}");

    ui.NetStatusLabel->setText("无法连接");
    ui.connectButton->setText("连接"); // 没有连接到任何网络，所以恢复到连接状态
    ui.connectButton->setEnabled(true); // 网络连接按钮恢复使用
    ui.changeSetting->setEnabled(false); // 控制器开关禁止使用
    ui.IP_ARM_Edit->setEnabled(true); // 可输入
    ui.Gateway_ARM_Edit->setEnabled(true); // 可输入
    ui.Port_ARM_Edit->setEnabled(true); // 可输入
}

// 连接成功，更新相应按钮功能
void ResetNetwork::connectUpdata()
{
    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(0,0,0);" //黑色
        "border: 2px solid rgb(54, 100, 139);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(0, 150, 136);"//橘红色
        "}");
    ui.NetStatusLabel->setText("连接成功");

    // 如果连接成功，实现按钮翻转
    ui.connectButton->setText("断开");
    ui.changeSetting->setEnabled(true); // 控制器开关可以使用
    ui.connectButton->setEnabled(true); // 网络连接按钮恢复使用
    ui.IP_ARM_Edit->setEnabled(true); //允许输入
    ui.Gateway_ARM_Edit->setEnabled(true); //允许输入
    ui.Port_ARM_Edit->setEnabled(true); //允许输入

    // 开启定时器查询继电器状态
    /*timer = new QTimer();
    timer->setInterval(500); //单位：ms
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut())); // 利用定时器发送指令查询状态
    */
}

// 断开连接，更新相应按钮功能
void ResetNetwork::disconnectUpdata()
{
    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(255,0,0);"//红色
        "border: 2px solid rgb(178, 34, 34);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(255, 165, 0);"
        "}");
    ui.NetStatusLabel->setText("未连接");

    // 如果断开连接，实现按钮翻转
    ui.connectButton->setText("连接");
    ui.changeSetting->setEnabled(false); // 控制器开关禁止使用
    ui.connectButton->setEnabled(true); // 网络连接按钮恢复使用
    ui.IP_ARM_Edit->setEnabled(true); // 可输入
    ui.Gateway_ARM_Edit->setEnabled(true); // 可输入
    ui.Port_ARM_Edit->setEnabled(true); // 可输入
}

// 修改配置
void ResetNetwork::on_changeSetting_clicked()
{
    QByteArray msg;
    msg.resize(12);
    msg[0] = 0x11; //包头
    msg[11] = 0x22; //包尾

    tcpIp = ui.IP_ARM_Edit->text();
    tcpGateway = ui.Gateway_ARM_Edit->text();
    tcpPort = ui.Port_ARM_Edit->text();

    // 确保输入的设备IP、设备网关是有效的。
    if (!ui.IP_ARM_Edit->isTextValid(tcpIp))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Warning");
        msgBox.setText("IP is in valid");
        msgBox.exec();
        return;
    }
    if (!ui.Gateway_ARM_Edit->isTextValid(tcpGateway))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Gateway is in valid");
        msgBox.exec();
        return;
    }

    QStringList ips = tcpIp.split(".");
    QStringList gateways = tcpGateway.split(".");
    msg[1] = ips.at(0).toInt();
    msg[2] = ips.at(1).toInt();
    msg[3] = ips.at(2).toInt();
    msg[4] = ips.at(3).toInt();
    msg[5] = gateways.at(0).toInt();
    msg[6] = gateways.at(1).toInt();
    msg[7] = gateways.at(2).toInt();
    msg[8] = gateways.at(3).toInt();

    int port = tcpPort.toInt();
    msg[9] = (unsigned char)(port / 256);
    msg[10] = (unsigned char)(port % 256);
    tcpSocket->write(msg);
}

// 定时器查询继电器状态
void ResetNetwork::onTimeOut()
{
    tcpSocket->write(tcp_order.PowerStatus); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
}

// 读取继电器发送的消息
void ResetNetwork::readMassage()
{
    QByteArray data = tcpSocket->readAll();//读取数据

    int ip1, ip2, ip3, ip4;
    int gw1, gw2, gw3, gw4;
    int port;

    ip1 = data.at(28) & 0xFF;
    ip2 = data.at(29) & 0xFF;
    ip3 = data.at(30) & 0xFF;
    ip4 = data.at(31) & 0xFF;

    gw1 = data.at(32) & 0xFF;
    gw2 = data.at(33) & 0xFF;
    gw3 = data.at(34) & 0xFF;
    gw4 = data.at(35) & 0xFF;

    int high, low;
    high = data.at(36) & 0xFF;
    low = data.at(37) & 0xFF;
    port = 256 * high + low;

    // tcpIp tcpGateway tcpPort
    QString str1 = QString::number(ip1) + '.' + QString::number(ip2) + '.'
        + QString::number(ip3) + '.' + QString::number(ip4);
    QString str2 = QString::number(gw1) + '.' + QString::number(gw2) + '.'
        + QString::number(gw3) + '.' + QString::number(gw4);
    QString str3 = QString::number(port);

    // 设备编号
    int EquipmentID = data.at(18) & 0xFF;

    if (!ui.IP_ARM_Edit->isTextValid(str1))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Warning");
        msgBox.setText("IP reset is in valid");
        msgBox.exec();
        return;
    }
    if (!ui.Gateway_ARM_Edit->isTextValid(str2))
    {
        QMessageBox msgBox;
        msgBox.setWindowTitle("Warning");
        msgBox.setText("Gateway reset is in valid");
        msgBox.exec();
        return;
    }

    QJsonObject jsonSetting = mainWindow::ReadSetting();
    jsonSetting["IP_Detector"] = str1;
    jsonSetting["Gateway_Detector"] = str2;
    jsonSetting["Port_Detector"] = str3;
    mainWindow::WriteSetting(jsonSetting);

    // 切断网络
    if (tcpSocket) {
        tcpSocket->abort();//abort函数用于使程序非正常中止/异常退出
    }

    QMessageBox msgBox;
    msgBox.setWindowTitle("参数设置");
    msgBox.setText("参数设置成功,新的参数如下：\nIP:" + str1 + "\n网关：" + str2 + "\n端口：" + str3);
    msgBox.exec();
    ui.connectButton->setText("连接"); // 设置成功后，网络失去连接，恢复到连接状态
    ui.changeSetting->setEnabled(false); // 控制器开关可以使用
    ui.connectButton->setEnabled(true); // 网络连接按钮恢复使用

    // 保存修改记录的日志
    QFile  m_pLogFile("./log/NetSet_Record.txt");
    if (!m_pLogFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)){
        qDebug() << "NetSet_Record.txt文件打开失败,无法写入日志";
        return;
    }
    QDateTime recordTime = QDateTime::currentDateTime();
    QString str_recordTime = recordTime.toString("yyyy-MM-dd hh:mm:ss");
    QTextStream in(&m_pLogFile);
    in << str_recordTime << QString(QStringLiteral("\n"))
        << QString(QStringLiteral("EquimentID: "))
        << QString::number(EquipmentID) << QString(QStringLiteral("\n"))
        << QString(QStringLiteral("IP is set to: "))
        << str1 << QString(QStringLiteral("\n"))
        << QString(QStringLiteral("Gateway is set to: "))
        << str2 << QString(QStringLiteral("\n"))
        << QString(QStringLiteral("Port is set to: "))
        << str3 << QString(QStringLiteral("\n\n"));
    m_pLogFile.close();
}

// 等待QTcpSocket写入数据
// 等待发送完毕，设置超时时间ms
void ResetNetwork::WaitingSocketWrite(int time) {
    if (!tcpSocket->waitForBytesWritten(time)) {
        return;
    }
}

// 响应关闭窗口动作，对一些数据进行销毁，以及ARM进行最后通信
void ResetNetwork::closeEvent(QCloseEvent* event)
{
    //if (tcpSocket) delete tcpSocket;
}