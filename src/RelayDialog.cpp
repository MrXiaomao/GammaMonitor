#include "RelayDialog.h"
#include "mainWindow.h"

#include <QElapsedTimer>

#pragma execution_character_set("utf-8") 

// 控制继电器的窗口
RelayDialog::RelayDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	// 读取配置文件：json文件
	QJsonObject jsonSetting = mainWindow::ReadSetting();
    tcpIp = jsonSetting["IP_Relay"].toString();
    tcpPort = jsonSetting["Port_Relay"].toString();
    
    ui.controlRelayButton->setEnabled(false);// 控制器开关禁止状态
    ui.IP_RelayEdit->setText(tcpIp);
    ui.Port_RelayEdit->setText(tcpPort);
    ui.Port_RelayEdit->setValidator(new QIntValidator(1, 9999, this));  // 端口号只能在[1,9999]范围内的整数输入
    timer = Q_NULLPTR;
    tcpSocket = Q_NULLPTR;//使用前先清空 
}

RelayDialog::~RelayDialog()
{
    //定时器关闭
    if (timer) {
        if (timer->isActive())//判断定时器是否在工作
            timer->stop();
        delete timer;
    }
    if(tcpSocket) delete tcpSocket;
}

void RelayDialog::on_connectRelayButton_clicked()
{
    if (ui.connectRelayButton->text() == "连接")
    {
        ui.connectRelayButton->setEnabled(false); // 连接按钮禁止使用，待系统响应网络
        ui.NetStatusLabel->setText("网络连接中。。。");

        if (tcpSocket) delete tcpSocket; //如果有指向其他空间直接删除
        tcpSocket = new QTcpSocket(this); //申请堆空间有TCP发送和接受操作

        // 点击连接按钮后，记录下当前IP以及Port
        tcpIp = ui.IP_RelayEdit->text();
        tcpPort = ui.Port_RelayEdit->text();
        QJsonObject jsonSetting = mainWindow::ReadSetting();
        jsonSetting["IP_Relay"] = tcpIp;
        jsonSetting["Port_Relay"] = tcpPort;
        mainWindow::WriteSetting(jsonSetting);

        if(!ui.IP_RelayEdit->isTextValid(tcpIp))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Warning");
            msgBox.setText("IP is in valid");
            msgBox.exec();
            return;
        }

        tcpSocket->connectToHost(tcpIp, tcpPort.toInt());//连接主机
        connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(displayError(QAbstractSocket::SocketError)));//错误连接
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectUpdata())); //更新连接之后按钮的使能
        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMassage())); //读取接收的信息
    }
    else if (ui.connectRelayButton->text() == "断开")
    {
        //定时器关闭
        if (timer->isActive())//判断定时器是否在工作
            timer->stop();
        if(timer) delete timer;

        tcpSocket->abort();//abort函数用于使程序非正常中止/异常退出
        if(tcpSocket) delete tcpSocket;
        tcpSocket = NULL;
        disconnectUpdata();
    }
}

// 在点击连接后，错误连接：即无法连接网络/失去连接，则进入该函数
void RelayDialog::displayError(QAbstractSocket::SocketError)
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
    ui.connectRelayButton->setText("连接"); // 没有连接到任何网络，所以恢复到连接状态
    ui.connectRelayButton->setEnabled(true); // 网络连接按钮恢复使用
    ui.controlRelayButton->setEnabled(false); // 控制器开关禁止使用
    ui.IP_RelayEdit->setEnabled(true); // 可输入
    ui.Port_RelayEdit->setEnabled(true); // 可输入
}

// 连接成功，更新相应按钮功能
void RelayDialog::connectUpdata()
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
    ui.connectRelayButton->setText("断开");
    ui.controlRelayButton->setEnabled(true); // 控制器开关可以使用
    ui.connectRelayButton->setEnabled(true); // 网络连接按钮恢复使用
    ui.IP_RelayEdit->setEnabled(false); //禁止输入
    ui.Port_RelayEdit->setEnabled(false); //禁止输入
    
    // 开启定时器查询继电器状态
    timer = new QTimer();
    timer->setInterval(500); //单位：ms
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut())); // 利用定时器发送指令查询状态

}

// 断开连接，更新相应按钮功能
void RelayDialog::disconnectUpdata()
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
    ui.connectRelayButton->setText("连接");
    ui.controlRelayButton->setEnabled(false); // 控制器开关禁止使用
    ui.connectRelayButton->setEnabled(true); // 网络连接按钮恢复使用
    ui.IP_RelayEdit->setEnabled(true);
    ui.Port_RelayEdit->setEnabled(true);
}

// 控制继电器开关
void RelayDialog::on_controlRelayButton_clicked()
{
    if (ui.controlRelayButton->text() == "关闭") {
        tcpSocket->write(tcp_order.PowerCH1_OFF); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
        tcpSocket->write(tcp_order.PowerCH2_OFF); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
    }
    else if(ui.controlRelayButton->text() == "打开") {
        tcpSocket->write(tcp_order.PowerCH1_ON); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
        tcpSocket->write(tcp_order.PowerCH2_ON); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
    }
}

// 定时器查询继电器状态
void RelayDialog::onTimeOut()
{
    tcpSocket->write(tcp_order.PowerStatus); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
}

// 读取继电器发送的消息
void RelayDialog::readMassage()
{
    QByteArray data = tcpSocket->readAll();//读取数据
    if (data == "00000000") {
        ui.label_5->setText("已打开");
        ui.controlRelayButton->setText("关闭");
    }
    if (data == "11000000") {
        ui.label_5->setText("已关闭");
        ui.controlRelayButton->setText("打开");
    }
    //ui->showLineEdit->setText(QString(data));//显示数据
}


// 等待QTcpSocket写入数据
// 等待发送完毕，设置超时时间ms
void RelayDialog::WaitingSocketWrite(int time) {
    if (!tcpSocket->waitForBytesWritten(time)) {
        return;
    }
}