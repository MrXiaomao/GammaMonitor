#include "mainWindow.h"
#include "ui_mainWindow.h"
#include <QFile>
#include <QJsonDocument>
#include <Qjsonarray>
#include <QVector>

#include <QNetworkProxy>

#include "RelayDialog.h"
#include "ResetNetwork.h"
#include "OpenFileDialog.h"
#include "ShowTXT.h"

#include <iostream>
#include <fstream>
#include <iomanip>
using namespace std;

#pragma execution_character_set("utf-8") 

mainWindow::mainWindow(QWidget *parent)
    : QMainWindow(parent),ui(new Ui::mainWindowClass)
{     
    ui->setupUi(this);

    // 变量初始化
    PackNumber = 0;
    plotCount = 0;
    timeLength = 0;
    MeasureStatus = false;
    TotalPackArray.clear();
    counter1.clear();
    counter2.clear();
    counter3.clear();
    counter4.clear();
    temperatue.clear();
    tcpSocket = Q_NULLPTR;//使用前先清空 
    timer = Q_NULLPTR;
    mTracer = TracerFlag::NoTracer;

    refreshPlotFlag = true;
    RescaleAxesFlag = true;

    tracerCross = Q_NULLPTR;
    lineTracer = Q_NULLPTR;
    for (int i = 0; i < 4; i++) {
        tracerX[i] = Q_NULLPTR;
    }

    // 给customPlot绘图控件，设置个别名，方便书写
    pPlot = ui->customPlot;

    initUI();
    
    // 状态栏指针
    sBar = statusBar();
    // 初始化图表1
    QPlot_init(pPlot);

    connect(this, SIGNAL(sigAppendMsg(const QString&, QtMsgType)), this, 
        SLOT(slotAppendMsg(const QString&, QtMsgType)));
}

mainWindow::~mainWindow()
{
    // 定时器关闭
    if (timer) {
        if (timer->isActive())// 判断定时器是否在工作
            timer->stop();
        delete timer;
    }
    if(tcpSocket) delete tcpSocket;
    delete ui;
}

void mainWindow::initUI()
{
    // 设置探测器编号示意图背景颜色为白色
    ui->gB_detSketchMap->setStyleSheet(
        "QGroupBox {"
        "background-color: white;"
        "}"
    );

    // 读取配置文件：json文件
    // IP地址和端口号
    QJsonObject jsonSetting = ReadSetting();
    LoadVoltageCoefficients();
    tcpIp = jsonSetting["IP_Detector"].toString();
    tcpPort = jsonSetting["Port_Detector"].toString();
    ui->widget_detIP->setIP(tcpIp);
    ui->Port_LineEdit->setText(tcpPort); 
    ui->Port_LineEdit->setValidator(new QIntValidator(1, 65536, this));  // 端口号只能在[1,65536]范围内的整数输入
    
    // 存储路径
    QString savePath = jsonSetting["SaveDir"].toString();
    if (!savePath.isEmpty()) {
        ui->le_savePath->setPlainText(savePath);
    }
    else {
        ui->le_savePath->setPlainText(QCoreApplication::applicationDirPath() + "/data"); // 默认路径为当前exe文件所在路径下的data文件夹
    }
    // 实验名称
    experimentName = jsonSetting["ExperimentName"].toString();
    if (!experimentName.isEmpty()) {
        ui->experimentNameEdit->setText(experimentName);
    }
    else {
        experimentName = "测试1";
        ui->experimentNameEdit->setText("测试1");
    }

    // 触发阈值
    int thresholdA = jsonSetting["ThresholdA"].toInt();
    int thresholdB = jsonSetting["ThresholdB"].toInt();
    ui->spinBox_thresholdA->setValue(thresholdA);
    ui->spinBox_thresholdB->setValue(thresholdB);

    ui->Measure_Button->setEnabled(false);//禁用状态
    QDateTime dateTime = QDateTime::currentDateTime();
    ui->measureTime_label->setText("无");
    ui->measrue_label->setText("无");
    ui->equipmentID_label->setText("无");

    // 布局/尺寸调整：让 customPlot 与 groupBox_4 高度占比为 2:1，保持 widget 和 widget_2 固定高度
    QWidget *leftContainer = ui->customPlot->parentWidget();
    if (leftContainer) {
        QBoxLayout *box = qobject_cast<QBoxLayout*>(leftContainer->layout());
        if (box) {
            const int idxPlot = box->indexOf(ui->customPlot);
            const int idxGroup = box->indexOf(ui->groupBox_4);
            if ((idxPlot != -1) && (idxGroup != -1)) {
                box->setStretch(idxPlot, 2);
                box->setStretch(idxGroup, 1);
            }
        }
    }

    // 将顶部的 widget 和底部的 widget_2 保持固定高度（使用它们的 sizeHint 作为固定值）
    if (ui->widget) {
        const int h = ui->widget->sizeHint().height();
        ui->widget->setFixedHeight(h);
    }
    if (ui->widget_2) {
        const int h2 = ui->widget_2->sizeHint().height();
        ui->widget_2->setFixedHeight(h2);
    }
}

// 读取配置文件，获取电压系数等参数
void mainWindow::LoadVoltageCoefficients()
{
    QJsonObject jsonSetting = ReadSetting();
    if (jsonSetting.isEmpty()) {
        qWarning() << "ReadSetting() returned empty JSON for voltage coefficients.";
        return;
    }

    const QJsonArray detectorIds = jsonSetting.value("detectorID").toArray();
    const QJsonArray coefA = jsonSetting.value("coef_DAC_2_VoltOfSiPMA").toArray();
    const QJsonArray coefB = jsonSetting.value("coef_DAC_2_VoltOfSiPMB").toArray();
    const QJsonArray coefInput = jsonSetting.value("coef_DAC_2_VoltOfInput").toArray();

    if (detectorIds.size() != coefA.size() || coefA.size() != coefB.size() || coefB.size() != coefInput.size()) {
        qWarning() << "Voltage coefficient arrays size mismatch:" \
                   << detectorIds.size() << coefA.size() << coefB.size() << coefInput.size();
        return;
    }

    coef_SiPMA_Volt.clear();
    coef_SiPMB_Volt.clear();
    coef_InputVolt.clear();

    for (int i = 0; i < detectorIds.size(); ++i) {
        const int detID = detectorIds.at(i).toInt();
        const double valueA = coefA.at(i).toDouble();
        const double valueB = coefB.at(i).toDouble();
        const double valueInput = coefInput.at(i).toDouble();
        coef_SiPMA_Volt[detID] = valueA;
        coef_SiPMB_Volt[detID] = valueB;
        coef_InputVolt[detID] = valueInput;
    }
}

// 绘图图表初始化
void mainWindow::QPlot_init(QCustomPlot* customPlot)
{
    // 图表添加两条曲线
    pGraph1_1 = customPlot->addGraph();
    pGraph1_2 = customPlot->addGraph();
    pGraph1_3 = customPlot->addGraph();
    pGraph1_4 = customPlot->addGraph();
    pGraphTotal = customPlot->addGraph();

    ui->checkBox1->setCheckState(Qt::Checked);  //设置复选框初始状态 Unchecked
    ui->checkBox2->setCheckState(Qt::Checked);  
    ui->checkBox3->setCheckState(Qt::Checked);  
    ui->checkBox4->setCheckState(Qt::Checked); 
    ui->cb_TotalCount->setCheckState(Qt::Unchecked);

    for (int i = 0; i < 4; i++) {
        isShowLine[i] = true;
    }
    isShowLine[4] = false;
    pGraphTotal->setVisible(false);

    ui->rescaleAxesCheckBox->setCheckState(Qt::Checked); // 坐标轴自适应
    ui->refreshPlotCheckBox->setCheckState(Qt::Checked); // 图像刷新
    
    ui->GetData_comboBox->setCurrentIndex(2);
    ui->TimeLen_ComboBox->setCurrentIndex(0);
    showTimeType = 0; // 绘图时长，全部时长，10min，5min,默认10min

    // 设置曲线颜色
    pGraph1_1->setPen(QPen(Qt::red));
    pGraph1_2->setPen(QPen(Qt::darkRed));
    pGraph1_3->setPen(QPen(Qt::green));
    pGraph1_4->setPen(QPen(Qt::blue));
    pGraphTotal->setPen(QPen(Qt::black));

    // 设置坐标轴名称
    customPlot->xAxis->setLabel("时间/s");
    customPlot->yAxis->setLabel("计数率/cps");

    // 设置x坐标轴显示范围
    customPlot->xAxis->setRange(0, 1000);
    customPlot->yAxis->setRange(0, 100);

    // 显示图表的图例
    customPlot->legend->setBrush(QColor(255, 255, 255, 0));//legend背景色设为白色但背景透明，允许图像在legend区域可见
    customPlot->legend->setVisible(true);

    // 添加曲线名称
    pGraph1_1->setName("探测器1");
    pGraph1_2->setName("探测器2");
    pGraph1_3->setName("探测器3");
    pGraph1_4->setName("探测器4");
    pGraphTotal->setName("总计数");

    // 设置波形曲线的复选框字体颜色
    ui->checkBox1->setStyleSheet("QCheckBox{color:red}");//设定前景颜色,就是字体颜色
    ui->checkBox2->setStyleSheet("QCheckBox{color:darkRed}");
    ui->checkBox3->setStyleSheet("QCheckBox{color:green}");
    ui->checkBox4->setStyleSheet("QCheckBox{color:blue}");
    ui->cb_TotalCount->setStyleSheet("QCheckBox{color:black}");
    
    // 允许用户用鼠标拖动轴范围，用鼠标滚轮缩放，点击选择图形:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //iRangeDrag 左键点击可拖动; iRangeZoom 范围可通过鼠标滚轮缩放; iSelectPlottables 线条可选中
}

void mainWindow::slotAppendMsg(const QString& msg, QtMsgType msgType)
{
    QTextCharFormat format;
    const QString ts = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz>>");
    QString logLine;

    if (msgType == QtWarningMsg) {
        format.setForeground(Qt::blue);
        logLine = QStringLiteral("%1 [WARN] %2").arg(ts).arg(msg);
    }
    else if (msgType == QtCriticalMsg || msgType == QtFatalMsg) {
        format.setForeground(Qt::red);
        logLine = QStringLiteral("%1 [ERROR] %2").arg(ts).arg(msg);
    }
    else {
        // QtDebugMsg、QtInfoMsg、QtSystemMsg 等：不打印级别字样
        logLine = QStringLiteral("%1 %2").arg(ts).arg(msg);
    }

    QTextCursor cursor = ui->plainTextEdit_log->textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(logLine, format);
    cursor.insertBlock();
    ui->plainTextEdit_log->setTextCursor(cursor);
}

// 连接网络/断开网络
void mainWindow::on_bt_connectDet_clicked()
{
    if (ui->bt_connectDet->text() == "连接网络")
    {
        qDebug() << "连接网络按钮被点击";
        ui->bt_connectDet->setEnabled(false); // 此时禁止用户点击
        //=====================创建测试数据的备份文件夹====================
        // 获取当前exe文件所在路径
        /*QString Filepath;
        Filepath = QCoreApplication::applicationDirPath();
        // 创建
        QString dirName = Filepath + "/" + "GMCOUNTER";
        QDir dir(dirName);
        if (!dir.exists()) {
            dir.mkdir(dirName);
            qDebug() << "GMCOUNTER文件夹创建成功";
        }*/
        
        //===========获取界面参数，并写入json文件==========
        tcpIp = ui->widget_detIP->getIP();
        tcpPort = ui->Port_LineEdit->text();
        if (tcpIp == NULL || tcpPort == NULL)//判断IP和PORT是否为空
        {
            /*QMessageBox::question(this, "bt_connectDet", "是否？");*/
            QMessageBox msgBox;
            msgBox.setWindowTitle("Warning");
            msgBox.setText("IP or PORT is Empty");
            msgBox.exec();
            return;
        }
        QJsonObject jsonSetting = ReadSetting();
        jsonSetting["IP_Detector"] = tcpIp;
        jsonSetting["Port_Detector"] = tcpPort;
        WriteSetting(jsonSetting);

        //===============连接网络==================
        if (tcpSocket) delete tcpSocket;    //如果有指向其他空间直接删除
        tcpSocket = new QTcpSocket(this);   //申请堆空间有TCP发送和接受操作
        // 避免因系统代理导致 "the proxy type is invalid for this operation"
        tcpSocket->setProxy(QNetworkProxy::NoProxy);
        connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(slotNetError(QAbstractSocket::SocketError)));  //错误连接
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectUpdata()));   //更新连接之后按钮的使能
        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMassage())); //读取信息的连接
        tcpSocket->connectToHost(tcpIp, tcpPort.toInt());   //连接主机
    }
    else if (ui->bt_connectDet->text() == "断开网络")
    {
        qDebug() << "断开网络按钮被点击";
        //============断网前最后通信，让ARM回到待机状态========
        if (tcpSocket) {
            ARM_Sleep();
            tcpSocket->abort();//abort函数用于使程序非正常中止/异常退出
            delete tcpSocket;
        }
        
        tcpSocket = Q_NULLPTR;
        disconnectUpdata();
    }
}

//继电器菜单栏响应
void mainWindow::on_relayMenu_triggered()
{
    RelayDialog* dialog = new RelayDialog();

    dialog->exec();	//如果是myDialod继承于QDialog，则使用该方法显示模态窗口								
    //dialog->show(); //如果是myDialod继承于QDialog，则使用该方法设置非模态窗口
}

// 菜单栏网络设置
void mainWindow::on_networkSettingMenu_triggered()
{
    if (tcpSocket) {
        QMessageBox::warning(this, tr("Warnning"), "请先在主界面断开网络后再进行网络设置");
        return;
    }
    
    qInfo().noquote() << "打开“修改网络配置”界面";
    ResetNetwork* dialog = new ResetNetwork();
    dialog->exec();	// 模态窗口						
    //dialog->show(); // 非模态窗口
}

// 打开文件
void mainWindow::on_openFileMenu_triggered(){
    QJsonObject jsonSetting = ReadSetting();
    QString saveDir = jsonSetting["SaveDir"].toString();
    // 判断是否存在默认路径，不存在则读取桌面所在目录。
    QDir dir(saveDir);
    if (!dir.exists()) {
        QStringList desktopLocation = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);//如果不存在该目录，则重定向到桌面所在目录
        saveDir = desktopLocation.at(0);
    }

    QString fileFullName = QFileDialog::getOpenFileName(this, "打开文件", saveDir, tr("TXT files(*.txt)"));
    // 判断文件名是否获取到
    if (!fileFullName.isEmpty()) {
        QFileInfo fileinfo = QFileInfo(fileFullName);
        QString file_path = fileinfo.absolutePath();//文件绝对路径

        jsonSetting["SaveDir"] = file_path;
        WriteSetting(jsonSetting);

        OpenFileDialog* dialog = new OpenFileDialog(fileFullName);
        
        dialog->show(); // 如果是myDialod继承于QDialog，则使用该方法设置非模态窗口
    }
}

// 打开帮助/网络修改日志 
void mainWindow::on_netLog_triggered() {
    // 获取可执行文件所在路径，eg:strDirPath = "/home/MonroeLiu/project/test/bin";
    QString strDirPath = QCoreApplication::applicationDirPath();
    QString fileFullName = strDirPath + "/log/NetSet_Record.txt";
    
    // 1、判断文件是否存在
    QFile file(fileFullName);
    QFileInfo fileInfo(file);
    if (!fileInfo.isFile()) {
        QString information = "文件：“" + fileFullName + "”不存在";
        QMessageBox::warning(NULL, "警告：文件不存在", fileFullName, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }
    ShowTXT* dialog = new ShowTXT("网络修改日志",fileFullName);
    dialog->show(); // 如果是myDialod继承于QDialog，则使用该方法设置非模态窗口
}

// 错误连接，在点击连接后，无法连接网络/失去连接，则进入该函数
void mainWindow::slotNetError(QAbstractSocket::SocketError)
{
    qWarning() << "探测器连接发生错误:" << tcpSocket->errorString();
    tcpSocket->close();
    tcpSocket = Q_NULLPTR;
    ui->Measure_Button->setEnabled(false);

    ui->connectStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(255,0,0);"//红色
        "border: 2px solid rgb(178, 34, 34);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(255, 165, 0);"
        "}");

    ui->connectStatusLabel->setText("无法连接");
    ui->bt_connectDet->setText("连接网络"); // 没有连接到任何网络，所以恢复到连接状态
    ui->bt_connectDet->setEnabled(true);
    ui->widget_detIP->setEnabled(true); // 可输入状态
    ui->Port_LineEdit->setEnabled(true); // 可输入状态
}

// 连接成功，更新相应按钮功能
void mainWindow::connectUpdata()
{
    qInfo().noquote() << "探测器连接成功";
    ui->connectStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(0,0,0);" //黑色
        "border: 2px solid rgb(54, 100, 139);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(0, 150, 136);"//橘红色
        "}");
    ui->connectStatusLabel->setText("连接成功");

    //================对单片机硬件初始化==================
    //-------------开启硬件电源---------
    // 探测器组A、组B以及外接设备开启电压
    int waitTime = tcp_order.waitingTime;
    if(tcpSocket->state() == QAbstractSocket::ConnectedState) {
        tcpSocket->write(tcp_order.DetecA_ON);  WaitingSocketWrite(); Sleep(waitTime);
        tcpSocket->write(tcp_order.DetecB_ON);  WaitingSocketWrite(); Sleep(waitTime);
        tcpSocket->write(tcp_order.ExtDeviceON);  WaitingSocketWrite(); Sleep(waitTime);

        //-------------设置比较器阈值-------------
        tcpSocket->write(tcp_order.DetectorThread);  WaitingSocketWrite(); Sleep(waitTime);

        //-------------对各个硬件状态监测开启（目前不考虑关闭监测）-------------
        tcpSocket->write(tcp_order.VoltageA_MonitorON); WaitingSocketWrite(); Sleep(waitTime);
        tcpSocket->write(tcp_order.VoltageB_MonitorON);  WaitingSocketWrite(); Sleep(waitTime);
        tcpSocket->write(tcp_order.InputVoltage_MonitorON);  WaitingSocketWrite(); Sleep(waitTime);
        tcpSocket->write(tcp_order.Temp_MonitorON);  WaitingSocketWrite(); Sleep(waitTime);
        
        //-------------开始检测ARM硬件电路的基本状态-------------
        tcpSocket->write(tcp_order.MonitorMessageON);  WaitingSocketWrite(); Sleep(waitTime);
    }
    else {
        qWarning() << "连接成功后，探测器状态异常，无法发送初始化指令";
    }
    // ==================连接成功，相关按钮翻转=================
    ui->bt_connectDet->setText("断开网络");
    ui->bt_connectDet->setEnabled(true);
    ui->Measure_Button->setEnabled(true);
    ui->widget_detIP->setEnabled(false); //禁止输入状态
    ui->Port_LineEdit->setEnabled(false);//禁止输入状态
}

// 断开连接，更新相应按钮功能
void mainWindow::disconnectUpdata()
{
    qInfo().nospace() << "探测器断开连接";
    // 状态信息恢复
    ui->VoltA_label->setText("无");
    ui->VoltB_label->setText("无");
    ui->InputVolt_label->setText("无");
    ui->temperature_label->setText("无");
    ui->measureTime_label->setText("无");
    ui->measrue_label->setText("无");
    ui->equipmentID_label->setText("无");

    ui->connectStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(255,0,0);"//红色
        "border: 2px solid rgb(178, 34, 34);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(255, 165, 0);"
        "}");
    ui->connectStatusLabel->setText("已断开连接");

    // 如果断开连接，实现按钮翻转
    ui->bt_connectDet->setText("连接网络");
    ui->widget_detIP->setEnabled(true);
    ui->Port_LineEdit->setEnabled(true);
}

//读取网口数据
void mainWindow::readMassage()
{
    if (!tcpSocket)
        return;

    TotalPackArray += tcpSocket->readAll();

    const int StandardPackLength = 40;
    // 无包头时避免 TotalPackArray 无限增长（异常流/断连残留）
    const int kMaxStickyBuffer = 4096;

    for (;;) {
        if (TotalPackArray.size() < 2)
            return;

        int HeadIndex = -1;
        for (int i = 0; i < TotalPackArray.size() - 1; ++i) {
            if ((TotalPackArray.at(i) & 0xFF) == 0xFF && (TotalPackArray.at(i + 1) & 0xFF) == 0xFE) {
                HeadIndex = i;
                break;
            }
        }
        if (HeadIndex == -1) {
            if (TotalPackArray.size() > kMaxStickyBuffer)
                TotalPackArray.remove(0, TotalPackArray.size() - 255);
            return;
        }
        if (HeadIndex > 0)
            TotalPackArray.remove(0, HeadIndex);

        // 包尾必须在包头之后查找，避免把包头前噪声里的 FF FD 当成帧尾
        int TailIndex = -1;
        for (int i = 2; i < TotalPackArray.size() - 1; ++i) {
            if ((TotalPackArray.at(i) & 0xFF) == 0xFF && (TotalPackArray.at(i + 1) & 0xFF) == 0xFD) {
                TailIndex = i;
                break;
            }
        }
        if (TailIndex == -1)
            return;

        const int frameBytes = TailIndex + 2;
        if (frameBytes != StandardPackLength) {
            // 长度不对：丢弃当前假包头，避免错位后整段数据作废
            TotalPackArray.remove(0, 2);
            continue;
        }

        const QByteArray OnePackArray = TotalPackArray.left(StandardPackLength);
        TotalPackArray.remove(0, StandardPackLength);

        PackNumber++;

        //-------------获取设备基本状态参数-------------
        EquipmentID = static_cast<quint8>(OnePackArray.at(18));
        ui->equipmentID_label->setNum(EquipmentID);

        double temp = GetTemperature(OnePackArray);
        ui->temperature_label->setText(QString::number(temp, 'f', 1));

        double voltInput = GetOuterVolt(OnePackArray);
        ui->InputVolt_label->setText(QString::number(voltInput, 'f', 1));

        double voltA = GetVolt_A(OnePackArray);
        ui->VoltA_label->setText(QString::number(voltA, 'f', 1));

        double voltB = GetVolt_B(OnePackArray);
        ui->VoltB_label->setText(QString::number(voltB, 'f', 1));

        if (MeasureStatus)
        {
            nowTime = QDateTime::currentDateTime();
            timeLength = beginTime.secsTo(nowTime);
            int hours = timeLength / 3600;
            int rest_seconds = timeLength - hours * 3600;
            int minutes = rest_seconds / 60;
            rest_seconds -= minutes * 60;
            QString str_Time;
            if (hours > 0)
                str_Time = QString::number(hours) + "h" + QString::number(minutes) + "min" + QString::number(rest_seconds);
            else if (minutes > 0)
                str_Time = QString::number(minutes) + "min" + QString::number(rest_seconds);
            else
                str_Time = QString::number(rest_seconds);
            ui->measrue_label->setText(str_Time);

            int counter[4];
            GetCounter(OnePackArray, counter);

            const int Num1 = counter[0];
            const int Num2 = counter[1];
            const int Num3 = counter[2];
            const int Num4 = counter[3];

            ui->label_count1->setNum(Num1);
            ui->label_count2->setNum(Num2);
            ui->label_count3->setNum(Num3);
            ui->label_count4->setNum(Num4);
            ui->label_TotalCount->setNum(Num1 + Num2 + Num3 + Num4);

            while (plotCount < timeLength)
            {
                counter1.push_back(Num1);
                counter2.push_back(Num2);
                counter3.push_back(Num3);
                counter4.push_back(Num4);
                temperatue.push_back(temp);
                plotCount++;
                Show_Plot(pPlot, Num1 * 1.0, Num2 * 1.0, Num3 * 1.0, Num4 * 1.0);
            }
            if (timeLength % 30 == 0)
            {
                SaveFile(autofilePath, counter1, counter2, counter3, counter4, temperatue);
            }
        }
    }
}

// 从ARM发来的数据中解析四个探测器的计数信息
// DataPack是不包含包头的数据包,包头2字节
void mainWindow::GetCounter(QByteArray DataPack, int* count)
{
    if (DataPack.size() < 16) {
        count[0] = count[1] = count[2] = count[3] = 0;
        return;
    }
    // 将16进制的QByteArray转化为十进制的int
    int i = 2; // DataPack 含包包头，从第2字节开始
    int detectorID1 = DataPack.at(i++) & 0xFF; // 探测器编号
    int high = DataPack.at(i++) & 0xFF;      // 高八位
    int middle = DataPack.at(i++) & 0xFF;   // 中八位
    int low = DataPack.at(i++) & 0xFF;      // 低八位
    count[0] = (high << 16) | (middle << 8) | low; // 24位大端组合
    
    int detectorID2 = DataPack.at(i++) & 0xFF;
    high = DataPack.at(i++) & 0xFF;
    middle = DataPack.at(i++) & 0xFF;
    low = DataPack.at(i++) & 0xFF;
    count[1] = (high << 16) | (middle << 8) | low;
    
    int detectorID3 = DataPack.at(i++) & 0xFF;
    high = DataPack.at(i++) & 0xFF;
    middle = DataPack.at(i++) & 0xFF;
    low = DataPack.at(i++) & 0xFF;
    count[2] = (high << 16) | (middle << 8) | low;

    int detectorID4 = DataPack.at(i++) & 0xFF;
    high = DataPack.at(i++) & 0xFF;
    middle = DataPack.at(i++) & 0xFF;
    low = DataPack.at(i++) & 0xFF;
    count[3] = (high << 16) | (middle << 8) | low;
}

// 获取温度
// DataPack是不包含包头的数据包,包头2字节
double mainWindow::GetTemperature(QByteArray DataPack)
{
    if (DataPack.size() < 20) {
        return 0.0;
    }
    // 读取温度信息（16位大端补码）
    quint8 high = static_cast<quint8>(DataPack.at(19));
    quint8 low = static_cast<quint8>(DataPack.at(20));
    unsigned short int temp = (high << 8) | low;

    double temperature = 0.0;
    if (temp & 0x8000) { // 负数：转换为原码
        unsigned short int original = (~temp) + 1;
        // 整数部分：位14~7 (8位)
        int int_part = (original >> 7) & 0xFF;
        // 小数部分：位6~0 (7位)
        int frac_part = original & 0x7F;
        temperature = -(int_part + frac_part / 128.0);
    } else { // 正数
        // 整数部分：位14~7 (8位)
        int int_part = (temp >> 7) & 0xFF;
        // 小数部分：位6~0 (7位)
        int frac_part = temp & 0x7F;
        temperature = int_part + frac_part / 128.0;
    }
    return temperature;
}

// 获取外部电压
// DataPack是不包含包头的数据包, 包头2字节
double mainWindow::GetOuterVolt(QByteArray DataPack)
{
    if (DataPack.size() < 23) {
        return 0.0;
    }
    quint8 high = static_cast<quint8>(DataPack.at(21));
    quint8 low  = static_cast<quint8>(DataPack.at(22));
    int outervoltage_adc = (high << 8) | low;
    
    double coefficient = 0.002762;
	auto it = coef_InputVolt.find(EquipmentID);
	if (it != coef_InputVolt.end()) {
		// 键存在
		coefficient = coef_InputVolt[EquipmentID];
	}

    double outervoltage = 0.0;
    outervoltage = outervoltage_adc * coefficient;	
    return outervoltage;
}

// 获取探测器A组偏压
// DataPack是不包含包头的数据包,包头2字节
double mainWindow::GetVolt_A(QByteArray DataPack)
{
    if (DataPack.size() < 25) {
        return 0.0;
    }
    quint8 high = static_cast<quint8>(DataPack.at(23));
    quint8 low  = static_cast<quint8>(DataPack.at(24));
    int sipmvoltege_A_adc = (high << 8) | low;

    double coefficient = 0.023297; //默认值
	auto it = coef_SiPMA_Volt.find(EquipmentID);
	if (it != coef_SiPMA_Volt.end()) {
		coefficient = coef_SiPMA_Volt[EquipmentID];
	}

    double sipmvoltege_A = 0.0;
	sipmvoltege_A = sipmvoltege_A_adc * coefficient;
    return sipmvoltege_A;
}

// 获取探测器B组偏压
// DataPack是不包含包头的数据包,包头2字节
double mainWindow::GetVolt_B(QByteArray DataPack)
{
    if (DataPack.size() < 27) {
        return 0.0;
    }

    quint8 high = static_cast<quint8>(DataPack.at(25));
    quint8 low  = static_cast<quint8>(DataPack.at(26));
    int sipmvoltege_B_adc = (high << 8) | low;
	
    double coefficient = 0.023297; //默认值
	auto it = coef_SiPMB_Volt.find(EquipmentID);
	if (it != coef_SiPMB_Volt.end()) {
		coefficient = coef_SiPMB_Volt[EquipmentID];
	}

    double sipmvoltege_B = 0.0;
	sipmvoltege_B = sipmvoltege_B_adc * coefficient;
    return sipmvoltege_B;
}

//开始测量&停止测量按钮
void mainWindow::on_Measure_Button_clicked() 
{
    if (ui->Measure_Button->text() == "开始测量")
    {
        //===============检查保存文件路径====================
        const QString saveDir = ui->le_savePath->toPlainText().trimmed();
        if (saveDir.isEmpty()) {
            QMessageBox::warning(this, tr("存储路径未设置"), tr("请先填写或选择保存目录。"));
            return;
        }
        const QDir outDir(saveDir);
        if (!outDir.exists()) {
            qWarning() << "存储路径不存在:" << saveDir;
            QMessageBox::warning(this, tr("存储路径不存在"), tr("存储路径 %1 不存在，请检查设置或选择其他路径。").arg(saveDir));
            return;
        }

        //===========清除上一次的测量数据============
        counter1.clear();
        counter2.clear();
        counter3.clear();
        counter4.clear();
        temperatue.clear();

        //==============清空绘图曲线的缓存数据==============
        int count = ui->customPlot->graphCount();//获取曲线条数
        for (int i = 0; i < count; ++i)
        {
            pPlot->graph(i)->data().data()->clear();
        }

        //====================重置部分变量，以及控件=====================
        //-----------变量-----------
        TotalPackArray.clear();
        PackNumber = 0; // 每次点击开始按钮，清空前一次的包个数
        plotCount = 0; // 绘图点个数重制
        timeLength = 0;
        MeasureStatus = true;
        refreshPlotFlag = true;
        //-----------控件-----------
        ui->refreshPlotCheckBox->setCheckState(Qt::Checked); // 图像刷新
        ui->spinBox_thresholdA->setEnabled(false);//禁止输入状态
        ui->spinBox_thresholdB->setEnabled(false);//禁止输入状态
        ui->le_savePath->setEnabled(false);//禁止输入状态
        ui->experimentNameEdit->setEnabled(false);//禁止输入状态

        // 设置触发阈值（两字节大端；显式类型避免 int→char 隐式窄化）
        const quint16 t1 = static_cast<quint16>(ui->spinBox_thresholdA->value());
        const quint16 t2 = static_cast<quint16>(ui->spinBox_thresholdB->value());

        QByteArray msg;
        msg.resize(6);
        msg[0] = static_cast<char>(0x50);
        msg[1] = static_cast<char>(0x01);
        msg[2] = static_cast<char>(static_cast<quint8>((t1 >> 8) & 0xFF));
        msg[3] = static_cast<char>(static_cast<quint8>(t1 & 0xFF));
        msg[4] = static_cast<char>(static_cast<quint8>((t2 >> 8) & 0xFF));
        msg[5] = static_cast<char>(static_cast<quint8>(t2 & 0xFF));
        
        if(tcpSocket!=Q_NULLPTR && tcpSocket->state() == QAbstractSocket::ConnectedState) {
            // PC端向ARM端发送设置比较器阈值指令
            tcpSocket->write(msg);
            WaitingSocketWrite(); 
            Sleep(tcp_order.waitingTime);
            // =========PC端向ARM端发送开始测量指令==============
            tcpSocket->write(tcp_order.StartMeasure);
        }
        else {
            qWarning() << "无法发送开始测量指令，探测器网络状态异常";
            QMessageBox::warning(this, tr("错误"), tr("无法发送开始测量指令，请检查探测器连接状态。"));
            return;
        }
        
        // ==============记录实验开始时间==================
        beginTime = QDateTime::currentDateTime();
        QString str_beginTime = beginTime.toString("yyyy-MM-dd hh:mm:ss");
        ui->measureTime_label->setText(str_beginTime);

        QString EquipmentID = ui->equipmentID_label->text();
        experimentName = ui->experimentNameEdit->toPlainText().trimmed();
        QString fileName = QString("设备%1_%2%3.txt")
            .arg(EquipmentID)
            .arg(experimentName.isEmpty() ? "unnamed" : experimentName)
            .arg(beginTime.toString("_yyyy-MM-dd_hh-mm-ss"));
        autofilePath = outDir.filePath(fileName);
        ui->Measure_Button->setText(QString("停止测量"));
        qInfo() << "开始测量";
        qInfo() << "触发阈值：" << t1 << " " << t2;
        qInfo() << "保存文件路径：" << autofilePath;
    }
    else if (ui->Measure_Button->text() == "停止测量")
    {
        MeasureStatus = false;
        // PC端向ARM端发送停止测量指令
        if(tcpSocket!=Q_NULLPTR && tcpSocket->state() == QAbstractSocket::ConnectedState) {
            tcpSocket->write(tcp_order.StopMeasure);
        }
        else {
            qWarning() << "无法发送停止测量指令，探测器网络状态异常";
            QMessageBox::warning(this, tr("错误"), tr("无法发送停止测量指令，请检查探测器连接状态。"));
            return;
        }
        
        // 保存测量数据
        if (counter1.size() > 0) {
            SaveFile(autofilePath, counter1, counter2, counter3, counter4, temperatue);//保存这次的测量数据至默认路径
        }
        
        // 恢复使用
        ui->Measure_Button->setText(QString("开始测量")); //按钮翻转
        ui->le_savePath->setEnabled(true);//恢复输入状态
        ui->experimentNameEdit->setEnabled(true);//恢复输入状态
        ui->spinBox_thresholdA->setEnabled(true);//恢复输入状态
        ui->spinBox_thresholdB->setEnabled(true);//恢复输入状态
        qInfo() << "停止测量";
    }
}

// 保存数据，覆盖式写入
void mainWindow::SaveFile(QString filepath, QVector<int>data1, QVector<int>data2,
    QVector<int>data3, QVector<int>data4, QVector<double>temp, int timeLen)
{
    //请注意，读写I/O口比较费时间，所以，最好隔一会儿存一次数据
    QString cstr_STARTTIME = beginTime.toString("yy-MM-dd hh:mm:ss");//文件中的内容

    // 中文路径乱码问题
    QTextCodec* code = QTextCodec::codecForName("UTF-8");
    QString filename = QString::fromStdString(code->fromUnicode(filepath).data());

    QFile  m_fFile(filename);
    if (!m_fFile.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        qDebug() << "open file fail";
        return;
    }
    {
        //关闭文件
        QTextStream in(&m_fFile);
        if (timeLen == 0) {
            in << "EqiumentID:" << EquipmentID << "   Begin Time:" << cstr_STARTTIME << "\n";
        }
        else {
            in << "EqiumentID:" << EquipmentID << "   Start Time:" << cstr_STARTTIME <<" Measure Time:"<< timeLen<<"s\n";
        }
        in << "time(s) data1 data2 data3 data4 temperature\n";
        int len1 = data1.size();
        for (int ii = 0; ii < len1; ii++)
        {
            in << ii + 1 << " " << data1[ii] << " " << data2[ii] << " " << data3[ii] << " " << data4[ii] << " " << QString::number(temp[ii], 'f', 1) << endl;
        }
        m_fFile.close();
    }
}

void mainWindow::PlotData(const QVector<double>& x, const QVector<double>& y, QColor color)
{
    //black         white        darkGray        gray        lightGray    red        green
    //blue        cyan        magenta    yellow        darkRed        darkGreen        darkBlue
    //darkCyan        darkMagenta        darkYellow
    ui->customPlot->addGraph();//添加一条曲线
    ui->customPlot->graph()->setData(x, y);//给曲线传递两个参数
    ui->customPlot->graph()->setPen(QPen(color));//设置曲线颜色

    ui->customPlot->xAxis->setLabel("x");//给曲线的横纵坐标命名
    ui->customPlot->yAxis->setLabel("y");
    
    ui->customPlot->xAxis2->setVisible(true);//显示上方X轴
    ui->customPlot->xAxis2->setTickLabels(false);//不显示上方X轴 刻度
    ui->customPlot->yAxis2->setVisible(true);//显示右侧Y轴
    ui->customPlot->yAxis2->setTickLabels(false);//不显示右侧Y轴 刻度          
    ui->customPlot->graph()->rescaleAxes(true); //自动调整坐标轴范围
    ui->customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);//放大拖拽选中等
    //iRangeDrag 左键点击可拖动; iRangeZoom 范围可通过鼠标滚轮缩放; iSelectPlottables 线条可选中
}

// 绘制曲线
// customPlot为绘图对象，num1~num4为要添加的4个探测器计数，也就是纵坐标
void mainWindow::Show_Plot(QCustomPlot* customPlot, double num1, double num2, double num3, double num4)
{
    // 给曲线添加数据
    pGraph1_1->addData(plotCount, num1);
    pGraph1_2->addData(plotCount, num2);
    pGraph1_3->addData(plotCount, num3);
    pGraph1_4->addData(plotCount, num4);
    pGraphTotal->addData(plotCount, num1 + num2 + num3 + num4);

    // 自动调节坐标轴
    if (RescaleAxesFlag){
        pGraph1_1->rescaleValueAxis(); // 让范围自行缩放，使图0完全适合于可见区域.这里不能带参数true
        pGraph1_2->rescaleValueAxis(); // 图1也是一样自动调整范围，但只是放大或不变范围
        pGraph1_3->rescaleValueAxis();
        pGraph1_4->rescaleValueAxis();
        if (isShowLine[4]) {
            pGraphTotal->rescaleValueAxis();
        }

        QCPRange yRange = customPlot->yAxis->range();
        double ySpan = yRange.upper - yRange.lower;
        double yMargin = ySpan * 0.05;
        if (yMargin <= 0) {
            double baseValue = (yRange.upper >= 0) ? yRange.upper : -yRange.upper;
            yMargin = (baseValue > 0) ? (baseValue * 0.05) : 1.0;
        }
        customPlot->yAxis->setRange(yRange.lower - yMargin, yRange.upper + yMargin);
    }
    // 设置x坐标轴显示范围，使其自适应缩放x轴，x轴最大显示1000个点
    customPlot->xAxis->setRange((pGraph1_1->dataCount() > 1000) ? (pGraph1_1->dataCount() - 1000) : 0, pGraph1_1->dataCount());

    // 更新绘图，这种方式在高填充下太浪费资源。有另一种方式rpQueuedReplot，可避免重复绘图。
    // 最好的方法还是将数据填充、和更新绘图分隔开。将更新绘图单独用定时器更新。例程数据量较少没用单独定时器更新，实际工程中建议大家加上。
    if (refreshPlotFlag) { adjustXRange(); customPlot->replot(QCustomPlot::rpQueuedReplot); }

    //=================计算帧数============
    static QTime time(QTime::currentTime());
    double key = time.elapsed() / 1000.0; // 开始到现在的时间，单位秒
    static double lastFpsKey;
    static int frameCount;
    ++frameCount;
    if (key - lastFpsKey > 1) // 每2秒求一次平均值
    {
        //状态栏显示帧数和数据总数
        sBar->showMessage(
        QString("%1 FPS, Total Data points: %2")
        .arg(frameCount / (key - lastFpsKey), 0, 'f', 0)
        .arg(pGraph1_1->data()->size())
        , 0);
        lastFpsKey = key;
        frameCount = 0;
    }
}

/// 隐藏曲线有两种方法：1.设置为透明色，但也会影响图例中的颜色    2.设置可见性属性
// 1. setPen设置为透明色的方法，隐藏曲线，但也会影响图例中的颜色。不建议使用。
// 2. setVisible设置可见性属性，隐藏曲线，不会对图例有任何影响。推荐使用。
// 复选框1

// 是否绘制曲线1
void mainWindow::on_checkBox1_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) {
        isShowLine[0] = true;
        pGraph1_1->setVisible(true);
        // 若存在曲线取值
        if (tracerX[0] != Q_NULLPTR) {
            tracerX[0]->setVisible(true);
        }
    }
    else {
        isShowLine[0] = false;
        pGraph1_1->setVisible(false);//void QCPLayerable::setVisible(bool on)
        // 若存在曲线取值
        if (tracerX[0] != Q_NULLPTR) {
            tracerX[0]->setVisible(false);
        }
    }
    pPlot->replot();
}

// 是否绘制曲线2
void mainWindow::on_checkBox2_stateChanged(int arg1)
{
    if (arg1) {
        isShowLine[1] = true;
        pGraph1_2->setVisible(true);
        // 若存在曲线取值
        if (tracerX[1] != Q_NULLPTR) {
            tracerX[1]->setVisible(true);
        }
    }
    else {
        isShowLine[1] = false;
        pGraph1_2->setVisible(false);//void QCPLayerable::setVisible(bool on)
        // 若存在曲线取值
        if (tracerX[1] != Q_NULLPTR) {
            tracerX[1]->setVisible(false);
        }
    }
    pPlot->replot();
}

// 是否绘制曲线3
void mainWindow::on_checkBox3_stateChanged(int arg1)
{
    if (arg1) {
        isShowLine[2] = true;
        pGraph1_3->setVisible(true);
        // 若存在曲线取值
        if (tracerX[2] != Q_NULLPTR) {
            tracerX[2]->setVisible(true);
        }
    }
    else {
        isShowLine[2] = false;
        pGraph1_3->setVisible(false);//void QCPLayerable::setVisible(bool on)
        // 若存在曲线取值
        if (tracerX[2] != Q_NULLPTR) {
            tracerX[2]->setVisible(false);
        }
    }
    pPlot->replot();
}

// 是否绘制曲线4
void mainWindow::on_checkBox4_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) { //选中
        isShowLine[3] = true;
        pGraph1_4->setVisible(true);
        // 若存在曲线取值
        if (tracerX[3] != Q_NULLPTR) {
            tracerX[3]->setVisible(true);
        }
    }
    else { //未选中
        isShowLine[3] = false;
        pGraph1_4->setVisible(false);//void QCPLayerable::setVisible(bool on)
        // 若存在曲线取值
        if (tracerX[3] != Q_NULLPTR) {
            tracerX[3]->setVisible(false);
        }
    }
    pPlot->replot();
}

void mainWindow::on_cb_TotalCount_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) { //选中
        isShowLine[4] = true;
        pGraphTotal->setVisible(true);
    }
    else { //未选中
        isShowLine[4] = false;
        pGraphTotal->setVisible(false);//void QCPLayerable::setVisible(bool on)
    }
    if (RescaleAxesFlag) {
        pGraph1_1->rescaleValueAxis();
        pGraph1_2->rescaleValueAxis();
        pGraph1_3->rescaleValueAxis();
        pGraph1_4->rescaleValueAxis();
        if (isShowLine[4]) {
            pGraphTotal->rescaleValueAxis();
        }
    }
    pPlot->replot();
}

// 鼠标左键点击图像取值
void mainWindow::SLOT_mouseTracetoCoord(QMouseEvent* event)
{
    switch (mTracer) {
    case TracerFlag::CrossTracer: {
        DoCrossTracer(event);
        break;
    }
    case TracerFlag::CurveTracer: {
        DoCurveTracer(event);
        break;
    }
    case TracerFlag::NoTracer: 
        break;
    default:
        break;
    }
}

// 曲线取值
void mainWindow::DoCurveTracer(QMouseEvent* event)
{
    //直线范围限制
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    //获取坐标,窗体鼠标的位置，不是曲线x轴的值
    int x_pos = event->pos().x();
    //    int y_pos = e->pos().y();
    // 
    //将鼠标坐标值换成曲线x轴的值
    int x_value = round(pPlot->xAxis->pixelToCoord(x_pos));
    // 总计数曲线不显示曲线取值
    for (int i = pPlot->graphCount() - 2; i >= 0; --i)
    {
        // 获取x轴值对应的曲线中的y轴值
        float y_value = pPlot->graph(i)->data()->at(x_value)->value;
        //定义标签格式
        QString tip;
        if (x_value > xLow && x_value<xUp && y_value>yLow && y_value < yUp) {   // 直线、游标范围限制
            lineTracer->updatePosition(x_value, y_value); //只需要绘制一次直线
            tracerX[i]->updatePosition(x_value, y_value);
            
            lineTracer->setVisible(isShowLine[i]);
            tracerX[i]->setVisible(isShowLine[i]);
            //定义标签格式
            QString tip;
            tip = QString::number(i+1) + ":(" + QString::number(x_value) + "," + QString::number(y_value) + ")";
            tracerX[i]->setText(tip);
        }
        else
        {
            lineTracer->setVisible(false);
            tracerX[i]->setVisible(false);
        }

        //更新曲线
        pPlot->replot(QCustomPlot::rpQueuedReplot);
    }
}

// 十字架取值
void mainWindow::DoCrossTracer(QMouseEvent* event)
{
    //直线范围限制
    double xLow = pPlot->xAxis->range().lower;
    double yLow = pPlot->yAxis->range().lower;
    double xUp = pPlot->xAxis->range().upper;
    double yUp = pPlot->yAxis->range().upper;

    double x = pPlot->xAxis->pixelToCoord(event->pos().x());
    double y2 = pPlot->yAxis->pixelToCoord(event->pos().y());

    if (x > xLow && x<xUp && y2>yLow && y2 < yUp) {   //直线、游标范围限制
        lineTracer->updatePosition(x, y2);
        tracerCross->updatePosition(x, y2);
        lineTracer->setVisible(true);
        tracerCross->setVisible(true);
        //定义标签格式
        QString tip;
        tip = QString::number(x, 'f', 2) + "," + QString::number(y2, 'f', 2);
        tracerCross->setText(tip);
    }
    else
    {
        lineTracer->setVisible(false);
        tracerCross->setVisible(false);
    }

    //更新曲线
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

//若当前对象com_index_string值发生改变则触发此函数
void mainWindow::on_TimeLen_ComboBox_currentIndexChanged(const QString& arg1)
{
    showTimeType = ui->TimeLen_ComboBox->currentIndex();
    
    adjustXRange();

    //更新曲线
    pPlot->replot(QCustomPlot::rpQueuedReplot);
}

//调整根据界面的显示时长类型调整坐标轴范围
void mainWindow::adjustXRange()
{
    // 设置x坐标轴显示范围类型
    int timeLength = 1000; //图像可显示的时间宽度
    if (showTimeType == 0) { //全部时长
        timeLength = 1000;
        double Xmin = 0;
        double Xmax = (pGraph1_1->dataCount() > timeLength) ? pGraph1_1->dataCount() : timeLength;
        pPlot->xAxis->setRange(Xmin, Xmax);
    }
    else {
        if (showTimeType == 1) { // 10min
            timeLength = 10 * 60;
        }
        else { // 5min
            timeLength = 5 * 60;
        }
        double Xmin = (pGraph1_1->dataCount() > timeLength) ? (pGraph1_1->dataCount() - timeLength) : 0;
        double Xmax = (pGraph1_1->dataCount() > timeLength) ? pGraph1_1->dataCount() : timeLength;
        pPlot->xAxis->setRange(Xmin, Xmax);
    }
}

//若当前对象com_index_string值发生改变则触发此函数
void mainWindow::on_GetData_comboBox_currentIndexChanged(const QString& arg1)
{
    //将当前选项名赋值给变量str，输出当前选项名
    QString str = ui->GetData_comboBox->currentText();
    if (str == "无") {
        mTracer = TracerFlag::NoTracer;
        // 删除浮标相关变量，指针置空
        if (tracerCross != Q_NULLPTR) {
            delete tracerCross;
            tracerCross = Q_NULLPTR;
        }
        for (int i = 0; i < 4; i++) {
            if (tracerX[i] != Q_NULLPTR) {
                delete tracerX[i];
                tracerX[i] = Q_NULLPTR;
            }
        }
        if (lineTracer != Q_NULLPTR) {
            delete lineTracer;
            lineTracer = Q_NULLPTR;
        }
        disconnect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }
    if (str == "十字光标") {
        mTracer = TracerFlag::CrossTracer;
        tracerCross = new myTracer(pPlot, pGraph1_1, DataTracer);
        if (lineTracer != Q_NULLPTR) {
            delete lineTracer;
            lineTracer = Q_NULLPTR;
        }
        for (int i = 0; i < 4; i++) {
            if (tracerX[i] != Q_NULLPTR) {
                delete tracerX[i];
                tracerX[i] = Q_NULLPTR;
            }
        }
        lineTracer = new myTracerLine(pPlot, myTracerLine::Both);//画十字交叉线
        connect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }
    if (str == "曲线取值") {
        mTracer = TracerFlag::CurveTracer;
        if (tracerCross != Q_NULLPTR) {
            delete tracerCross;
            tracerCross = Q_NULLPTR;
        }
        if (lineTracer != Q_NULLPTR) {
            delete lineTracer;
            lineTracer = Q_NULLPTR;
        }

        for (int i = 0; i < 4; i++) {
            tracerX[i] = new myTracer(pPlot, pPlot->graph(i), DataTracer);
        }
        lineTracer = new myTracerLine(pPlot, myTracerLine::VerticalLine);//画垂直线
        connect(pPlot, SIGNAL(mousePress(QMouseEvent*)), this, SLOT(SLOT_mouseTracetoCoord(QMouseEvent*)));
    }
    
    adjustXRange();
    pPlot->replot();
}

//响应图像刷新复选框
void mainWindow::on_refreshPlotCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) { //选中
        refreshPlotFlag = true;
    }
    else { //未选中
        refreshPlotFlag = false;
    }
    adjustXRange();
    pPlot->replot();
}

//响应坐标轴自适应复选框
void mainWindow::on_rescaleAxesCheckBox_stateChanged(int arg1)
{
    if (arg1 == Qt::Checked) { //选中
        RescaleAxesFlag = true;
        pGraph1_1->rescaleAxes(); // 让范围自行缩放，使图0完全适合于可见区域.这里不能带参数true
        pGraph1_2->rescaleAxes(); // 图1也是一样自动调整范围，但只是放大或不变范围
        pGraph1_3->rescaleAxes();
        pGraph1_4->rescaleAxes();
    }
    else { //未选中
        RescaleAxesFlag = false;
        pGraph1_1->rescaleAxes(true);  //true 表示 只扩展坐标轴范围，不覆盖前面已经算好的范围。
        pGraph1_2->rescaleAxes(true); 
        pGraph1_3->rescaleAxes(true);
        pGraph1_4->rescaleAxes(true);
    }
    adjustXRange();
    pPlot->replot();
}

// 关闭窗口响应事件，弹出对话框：是/否/取消
// 该函数在析构函数之前运行
void mainWindow::closeEvent(QCloseEvent* event)
{
    int ret = QMessageBox::question(this, "关闭窗口", "是否退出",
        QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
        QMessageBox::Yes); // 默认接受退出
    switch (ret)
    {
        case QMessageBox::Yes:
            //slotSaveFile();
            closeAction();
            break;
        case QMessageBox::Cancel:
            event->ignore();//忽略退出事件，程序继续运行
            break;
        default:
            event->ignore();//忽略退出事件，程序继续运行
            break;
    }
    //event->accept();  //默认是接受退出事件，程序退出，所以这句可以忽略
}

// 响应关闭窗口动作，对一些数据进行销毁，以及ARM进行最后通信
void mainWindow::closeAction()
{
    //保存界面参数
    // 存储路径
    QString uiPath = ui->le_savePath->toPlainText().trimmed();
    QJsonObject jsonSetting = ReadSetting();
    if (!uiPath.isEmpty()) {
        jsonSetting["SaveDir"] = uiPath;
    }

    //实验文件名
    QString experimentName = ui->experimentNameEdit->toPlainText().trimmed();
    if (!experimentName.isEmpty()) {
        jsonSetting["ExperimentName"] = experimentName;
    }

    //触发阈值
    int thresholdA = ui->spinBox_thresholdA->value();
    int thresholdB = ui->spinBox_thresholdB->value();
    jsonSetting["ThresholdA"] = thresholdA;
    jsonSetting["ThresholdB"] = thresholdB;
    WriteSetting(jsonSetting);

    // 关闭前处于测量状态，PC端向ARM端发送停止测量指令
    if (ui->Measure_Button->text() == "停止测量")
    {
        if(tcpSocket!=Q_NULLPTR && tcpSocket->state() == QAbstractSocket::ConnectedState) {
            tcpSocket->write(tcp_order.StopMeasure);  WaitingSocketWrite();
        }
        // 延时关闭窗口，以确保网口能够把指令发送给ARM
        QTime t;
        t.start();
        while (t.elapsed() < 100) //单位ms,
            QCoreApplication::processEvents();
    }
    ARM_Sleep();
}

// 让ARM进入休眠，
// 关闭探测器A组电压、B组电压、外接设备电压
// 停止比较器工作
// 停止电压监测、温度监测
// 让ARM停止发送设备状态信息（温度、输入电源、探测器A组电压、探测器B组电压）
void mainWindow::ARM_Sleep()
{
    if(tcpSocket!=Q_NULLPTR && tcpSocket->state() == QAbstractSocket::ConnectedState) {
        int waitTime = tcp_order.waitingTime;
        tcpSocket->write(tcp_order.DetecA_OFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭探测器A组电压
        tcpSocket->write(tcp_order.DetecB_OFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭探测器B组电压
        tcpSocket->write(tcp_order.ExtDeviceOFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭外接设备电压

        tcpSocket->write(tcp_order.DetectorThreadOFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭比较器

        tcpSocket->write(tcp_order.VoltageA_MonitorOFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭A组偏压监测
        tcpSocket->write(tcp_order.VoltageB_MonitorOFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭B组偏压监测
        tcpSocket->write(tcp_order.InputVoltage_MonitorOFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭5V电压监测
        tcpSocket->write(tcp_order.Temp_MonitorOFF); WaitingSocketWrite(); Sleep(waitTime); // 关闭温度监测

        tcpSocket->write(tcp_order.MonitorMessageOFF); WaitingSocketWrite(); Sleep(waitTime);  // 让ARM停止发送数据
    }
}

// 等待QTcpSocket写入数据
// 等待发送完毕，设置超时时间ms
void mainWindow::WaitingSocketWrite(int time) {
    if (!tcpSocket->waitForBytesWritten(time)) {
        return;
    }
}

// 选择存储路径
void mainWindow::on_savePathButton_clicked()
{
    QString cacheDir = QFileDialog::getExistingDirectory(this);
    if (!cacheDir.isEmpty()) {
        ui->le_savePath->setPlainText(cacheDir);
    }
}

// 读取配置文件
QJsonObject mainWindow::ReadSetting()
{
    // 读取文件
    QFile file("./config/setting.json");
    file.open(QFile::ReadOnly);
    QByteArray all = file.readAll();
    file.close();

    QJsonDocument doc = QJsonDocument::fromJson(all);//转换成文档对象
    QJsonObject obj;
    if (doc.isObject())//可以不做格式判断，因为，解析的时候已经知道是什么数据了
    {
        obj = doc.object(); //得到Json对象
    }
    return obj;
}

// 写入配置文件，实际上是修改配置文件
void mainWindow::WriteSetting(QJsonObject myJson)
{
    //创建QJsonDocument对象并将根对象传入
    QJsonDocument jDoc(myJson);
    //打开存放json串的文件
    QFile file("./config/setting.json");
    if (!file.open(QIODevice::WriteOnly)) return ;

    //使用QJsonDocument的toJson方法获取json串并保存到数组
    QByteArray data(jDoc.toJson());
    //将json串写入文件
    file.write(data);
    file.close();
}

void mainWindow::GetConfig()
{
  QJsonObject jsonSetting = ReadSetting();

}
