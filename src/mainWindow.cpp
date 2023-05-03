#include "mainWindow.h"
#include "ui_mainWindow.h"
#include <QFile>
#include <QJsonDocument>
#include <Qjsonarray>
#include <QVector>

#include "RelayDialog.h"
#include "ResetNetwork.h"
#include "SetThreshlod.h"
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
    //this->setWindowIcon(QIcon(":/images/nuclear.ico"));

    //======================窗口初始化==============================
    experimentName = "测试1";
    autofilePath = "";
    ui->experimentNameEdit->setText(experimentName);

    // 读取配置文件：json文件
    QJsonObject jsonSetting = ReadSetting();
    tcpIp = jsonSetting["IP_Detector"].toString();
    tcpPort = jsonSetting["Port_Detector"].toString();
    ui->IP_LineEdit->setText(tcpIp);
    ui->Port_LineEdit->setText(tcpPort); 
    ui->Port_LineEdit->setValidator(new QIntValidator(1, 9999, this));  // 端口号只能在[1,9999]范围内的整数输入

    ui->Measure_Button->setEnabled(false);//禁用状态
    QDateTime dateTime = QDateTime::currentDateTime();
    ui->measureTime_label->setText("无");
    ui->measrue_label->setText("无");
    ui->equipmentID_label->setText("无");

    // 变量初始化
    PackNumber = 0;
    plotCount = 0;
    timeLength = 0;
    MeasureStaus = false;
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

    // 状态栏指针
    sBar = statusBar();
    // 初始化图表1
    QPlot_init(pPlot);
    //m_pCpTip = new QCPToolTip(ui->customPlot);
    //connect(pPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(myMoveMouseEvent(QMouseEvent*)));
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
    /*
    if (doc.isObject())//可以不做格式判断，因为，解析的时候已经知道是什么数据了
    {
        QJsonObject obj = doc.object(); //得到Json对象
        //读取字符串
        QString strVal = obj["NAME"].toString();
        //读取数值(对应的数值转换成对应的类型)
        int numVal = obj["Age"].toInt();
        //读取逻辑值
        bool boolVal = obj["IsLive"].toBool();
        qDebug() << "NAME" << ":" << strVal;
        qDebug() << "Age" << ":" << numVal;

        /*QStringList keys = obj.keys(); //得到所有key
        for (int i = 0; i < keys.size(); i++)
        {
            QString key = keys.at(i);
            QJsonValue value = obj.value(key);
            QString name = value.toString();
            if (value.isBool())
            {
                qDebug() << key << ":" << value.toBool();
            }
            else if (value.isString())
            {
                qDebug() << key << ":" << value.toString();
            }
            else if (value.isDouble())
            {
                qDebug() << key << ":" << value.toVariant().toInt();
            }
            else if (value.isObject())
            {
                qDebug() << key << ":";
                QJsonObject subObj = value.toObject();
                QStringList subKeys = subObj.keys();
                for (int k = 0; k < subKeys.size(); ++k)
                {
                    QJsonValue subValue = subObj.value(subKeys.at(k));
                    if (subValue.isString())
                    {
                        qDebug() << " " << subKeys.at(k) << ":" << subValue.toString();
                    }
                    else if (subValue.isArray())
                    {
                        qDebug() << " " << subKeys.at(k);
                        QJsonArray array = subValue.toArray();
                        for (int j = 0; j < array.size(); j++)
                        {
                            qDebug() << " " << array[j].toString();
                        }
                    }
                }
            }
        }
    }*/
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

// 绘图图表初始化
void mainWindow::QPlot_init(QCustomPlot* customPlot)
{
    // 图表添加两条曲线
    pGraph1_1 = customPlot->addGraph();
    pGraph1_2 = customPlot->addGraph();
    pGraph1_3 = customPlot->addGraph();
    pGraph1_4 = customPlot->addGraph();

    ui->checkBox1->setCheckState(Qt::Checked);  //设置复选框初始状态 Unchecked
    ui->checkBox2->setCheckState(Qt::Checked);  
    ui->checkBox3->setCheckState(Qt::Checked);  
    ui->checkBox4->setCheckState(Qt::Checked); 
    for (int i = 0; i < 4; i++) {
        isShowLine[i] = true;
    }

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

    // 设置波形曲线的复选框字体颜色
    ui->checkBox1->setStyleSheet("QCheckBox{color:red}");//设定前景颜色,就是字体颜色
    ui->checkBox2->setStyleSheet("QCheckBox{color:darkRed}");
    ui->checkBox3->setStyleSheet("QCheckBox{color:green}");
    ui->checkBox4->setStyleSheet("QCheckBox{color:blue}");
    
    // 允许用户用鼠标拖动轴范围，用鼠标滚轮缩放，点击选择图形:
    customPlot->setInteractions(QCP::iRangeDrag | QCP::iRangeZoom);
    //iRangeDrag 左键点击可拖动; iRangeZoom 范围可通过鼠标滚轮缩放; iSelectPlottables 线条可选中
}

// 连接网络/断开网络
void mainWindow::on_connectButton_clicked()
{
    if (ui->connectButton->text() == "连接网络")
    {
        ui->connectButton->setEnabled(false); // 此时禁止用户点击
        //=====================创建测试数据的备份文件夹====================
        // 获取当前exe文件所在路径
        QString Filepath;
        Filepath = QCoreApplication::applicationDirPath();
        // 创建
        QString dirName = Filepath + "/" + "GMCOUNTER";
        QDir dir(dirName);
        if (!dir.exists()) {
            dir.mkdir(dirName);
            qDebug() << "GMCOUNTER文件夹创建成功";
        }
        
        //===========获取界面参数，并写入json文件==========
        tcpIp = ui->IP_LineEdit->text();
        tcpPort = ui->Port_LineEdit->text();
        if (tcpIp == NULL || tcpPort == NULL)//判断IP和PORT是否为空
        {
            /*QMessageBox::question(this, "connectButton", "是否？");*/
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
        tcpSocket->connectToHost(tcpIp, tcpPort.toInt());   //连接主机
        connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(displayError(QAbstractSocket::SocketError)));  //错误连接
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectUpdata()));   //更新连接之后按钮的使能
        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMassage())); //读取信息的连接
    }
    else if (ui->connectButton->text() == "断开网络")
    {
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
    ResetNetwork* dialog = new ResetNetwork();

    dialog->exec();	//如果是myDialod继承于QDialog，则使用该方法显示模态窗口								
    //dialog->show(); //如果是myDialod继承于QDialog，则使用该方法设置非模态窗口
}

// 菜单栏探测器触发阈值设置
void mainWindow::on_setThresholdMenu_triggered()
{
    if (!tcpSocket) {
        QMessageBox::warning(this, tr("Warnning"), "请先在主界面\"连接网络\"后再进行探测器触发阈值设置");
        return;
    }
    SetThreshlod* dialog = new SetThreshlod(tcpSocket);
    
    dialog->exec();	//如果是myDialod继承于QDialog，则使用该方法显示模态窗口								
    //dialog->show(); //如果是myDialod继承于QDialog，则使用该方法设置非模态窗口
}

// 打开文件
void mainWindow::on_openFileMenu_triggered(){
    QJsonObject jsonSetting = ReadSetting();
    QString saveDir = jsonSetting["SaveDir"].toString(); //"SaveDir": "/home"
    // 判断是否存在默认路径，不存在则读取桌面所在目录。
    QDir dir(saveDir);
    if (!dir.exists()) {
        QStringList desktopLocation = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);//如果不存在该目录，则重定向到桌面所在目录
        saveDir = desktopLocation.at(0);
    }

    //QString fileFullName = QFileDialog::getSaveFileName(this, tr("打开文件"),
    //    saveDir,tr("TXT files(*.txt)"));
    QString fileFullName = QFileDialog::getOpenFileName(this, "打开文件", saveDir, tr("TXT files(*.txt)"));
    // 判断文件名是否获取到
    if (!fileFullName.isEmpty()) {
        QFileInfo fileinfo = QFileInfo(fileFullName);
        //QString file_name = fileinfo.fileName();//文件名称
        //QString file_suffix = fileinfo.suffix();//文件后缀格式
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
void mainWindow::displayError(QAbstractSocket::SocketError)
{
    QMessageBox::warning(this, tr("Warnning"), tcpSocket->errorString());
    tcpSocket->close(); 
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
    ui->connectButton->setText("连接网络"); // 没有连接到任何网络，所以恢复到连接状态
    ui->connectButton->setEnabled(true);
    ui->IP_LineEdit->setEnabled(true); // 可输入状态
    ui->Port_LineEdit->setEnabled(true); // 可输入状态
}

// 连接成功，更新相应按钮功能
void mainWindow::connectUpdata()
{
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
    
    // ==================连接成功，相关按钮翻转=================
    ui->connectButton->setText("断开网络");
    ui->connectButton->setEnabled(true);
    ui->Measure_Button->setEnabled(true);
    ui->IP_LineEdit->setEnabled(false); //禁止输入状态
    ui->Port_LineEdit->setEnabled(false);//禁止输入状态
}

// 断开连接，更新相应按钮功能
void mainWindow::disconnectUpdata()
{
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
    ui->Measure_Button->setEnabled(false);//禁止状态
    ui->IP_LineEdit->setEnabled(true); // 可输入状态
    ui->Port_LineEdit->setEnabled(true); // 可输入状态

    // 如果断开连接，实现按钮翻转
    ui->connectButton->setText("连接网络");
    ui->IP_LineEdit->setEnabled(true);
    ui->Port_LineEdit->setEnabled(true);
}

//读取网口数据
void mainWindow::readMassage()
{
    // 从接收缓冲区中读取数据
    QByteArray buffer = tcpSocket->readAll();

    // 一个包40个字节
    int StandardPackLength = 40;
    TotalPackArray += buffer;
    if (TotalPackArray.size() >= StandardPackLength)
    {
        //----------------------------------寻找包头包尾---------------------------------//
        int HeadIndex = -1; // 赋初值在0-258之外
        int TailIndex = -1;

        // DataHead FF FE 
        for (int i = 0; i < TotalPackArray.size() - 1; i++)
        {  
            if ((TotalPackArray.at(i) & 0xFF) == 0xFF)
                if ((TotalPackArray.at(i + 1) & 0xFF) == 0xFE)
                {
                    HeadIndex = i;
                    break;
                }
        }

        // DataTail FF FD
        for (int i = 0; i < TotalPackArray.size() - 1; i++)
        {
            if ((TotalPackArray.at(i) & 0xFF) == 0xFF)
                if ((TotalPackArray.at(i + 1) & 0xFF) == 0xFD)
                {
                    TailIndex = i;
                    break;
                }
        }

        //-----------------------数据包异常处理------------------------//
        if ((HeadIndex == -1) || (TailIndex == -1))  return; // 如果没有检测到包头或者包尾则返回。不执行后面语句

        if (HeadIndex > TailIndex) // 如果包头大于包尾则清除包头之前的数据
        {
            TotalPackArray.remove(0, HeadIndex);
            return;
        }

        // 先提取一个指定长度的数据包，不要包头包尾，假定是一个完整的数据包
        QByteArray OnePackArray = TotalPackArray.mid(HeadIndex+2, TailIndex - HeadIndex-2); // mid(startID,length)
        int OnePackDataLen = OnePackArray.size();

        TotalPackArray.remove(0, TailIndex + 2); // 清除该部分数据
        if (OnePackDataLen == StandardPackLength - 4)  // 包尾包尾各是两个字节，所以减去4
        {
            PackNumber++;  
            
            //-------------获取设备基本状态参数-------------
            // 获取温度
            double temp = GetTemperature(OnePackArray);

            ui->temperature_label->setText(QString::number(temp, 'f', 1));

            // 获取输入电压
            double voltInput = GetOuterVolt(OnePackArray);
            ui->InputVolt_label->setText(QString::number(voltInput, 'f', 1));

            // 获取探测器A组供电电压（SiPM电压）
            double voltA = GetVolt_A(OnePackArray);
            ui->VoltA_label->setText(QString::number(voltA, 'f', 1));

            // 获取探测器B组供电电压（SiPM电压）
            double voltB = GetVolt_B(OnePackArray);
            ui->VoltB_label->setText(QString::number(voltB, 'f', 1));

            // -------------获取探测器计数---------------
            if (MeasureStaus)
            {
                // -------计算时间差--------
                nowTime = QDateTime::currentDateTime();
                timeLength = beginTime.secsTo(nowTime);
                int hours = timeLength / 3600;
                int rest_seconds = timeLength - hours * 3600;
                int minutes = rest_seconds / 60;
                rest_seconds -= minutes * 60;
                QString str_Time = "";
                if (hours > 0)  str_Time = QString::number(hours) + "h" + QString::number(minutes) + "min" + QString::number(rest_seconds) + "s";
                else if (minutes > 0) str_Time = QString::number(minutes) + "min" + QString::number(rest_seconds) + "s";
                else str_Time = QString::number(rest_seconds) + "s";
                ui->measrue_label->setText(str_Time);

                // -------解析探测器计数数据------
                int counter[4];
                GetCounter(OnePackArray,counter);

                int Num1, Num2, Num3, Num4;
                Num1 = counter[0];
                Num2 = counter[1];
                Num3 = counter[2];
                Num4 = counter[3];

                ui->Conuter1Label->setNum(Num1);
                ui->Conuter2Label->setNum(Num2);
                ui->Conuter3Label->setNum(Num3);
                ui->Conuter4Label->setNum(Num4);

                // 当绘图点数少于时间，填充数据，若还是少于，则再次补一个数据，确保绘图点数与时间长度相等。
                // 否则不进行动作，也就是不往容器里存入数据，也不往绘图曲线中存入数据
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
                // 自动保存数据 每镉10秒保存一次数据，防止软件意外崩溃，
				//    因为保存数据(读写I/O口)比较费时间，所以不能每秒都进行保存，
                if (timeLength % 30 == 0)
                {   
                    SaveFile(autofilePath, counter1, counter2, counter3, counter4, temperatue);//保存这次的测量数据至默认路径
                }
            }
            
            // 设备编号
            EquipmentID = OnePackArray[18 - 2];
            ui->equipmentID_label->setNum(EquipmentID);
        }
    }
}

// 从ARM发来的数据中解析四个探测器的计数信息
// DataPack是不包含包头的数据包,包头2字节
void mainWindow::GetCounter(QByteArray DataPack, int* count)
{
    // 将16进制的QByteArray转化为十进制的int
    int i = 0;
    int detectorID1 = DataPack.at(i++) & 0xFF; // 探测器编号
    int high = DataPack.at(i++) & 0xFF;      // 转化高八位
    int middle = DataPack.at(i++) & 0xFF; // 转换中八位
    int low = DataPack.at(i++) & 0xFF;   // 转化低八位
    count[0] = high * 16 * 16 * 16 + middle * 16 * 16 + low;
    
    int detectorID2 = DataPack.at(i++) & 0xFF;
    high = DataPack.at(i++) & 0xFF;
    middle = DataPack.at(i++) & 0xFF;
    low = DataPack.at(i++) & 0xFF;
    count[1] = high * 16 * 16 * 16 + middle * 16 * 16 + low;
    
    int detectorID3 = DataPack.at(i++) & 0xFF;
    high = DataPack.at(i++) & 0xFF;
    middle = DataPack.at(i++) & 0xFF;
    low = DataPack.at(i++) & 0xFF;
    count[2] = high * 16 * 16 * 16 + middle * 16 * 16 + low;

    int detectorID4 = DataPack.at(i++) & 0xFF;
    high = DataPack.at(i++) & 0xFF;
    middle = DataPack.at(i++) & 0xFF;
    low = DataPack.at(i++) & 0xFF;
    count[3] = high * 16 * 16 * 16 + middle * 16 * 16 + low;
}

// 获取温度
// DataPack是不包含包头的数据包,包头2字节
double mainWindow::GetTemperature(QByteArray DataPack)
{
    // 读取温度信息
    double temperature = 0.0;
    unsigned short int temp = DataPack.at(19 - 2) * 16*16 + DataPack.at(20 - 2); 
    if (temp & 0x8000)
    {
        unsigned short int dl = ((~temp) | 0x8000);
        dl += 0x0001;
        float dat = 0;
        for (int i = 0; i < 15; i++)
        {
            if (((dl) >> i) | 0x0001)
            {
                dat += (-1) * (pow(2, i - 7));
            }
        }
        temperature = dat;
    }
    else
    {
        unsigned short int dl = temp;
        float dat = 0;
        for (int i = 0; i < 15; i++)
        {
            if (((dl) >> i) & 0x0001)
            {
                dat += (+1) * pow(2, i - 7);
            }
        }
        temperature = dat;
    }
    return temperature;
}

// 获取外部电压
// DataPack是不包含包头的数据包, 包头2字节
double mainWindow::GetOuterVolt(QByteArray DataPack)
{
    double outervoltage = 0.0;
    int outervoltage_adc = 256 * DataPack[21-2] + DataPack[22-2]; //减去包头两个字节
    //outervoltage = (outervoltage_adc * 3.3) / 4096;
    //outervoltage = outervoltage * 5;	//需要注意，这里的分压系数还需要实验来进行确定
    outervoltage = outervoltage_adc * 0.005421;	
    return outervoltage;
}

// 获取探测器A组偏压
// DataPack是不包含包头的数据包,包头2字节
double mainWindow::GetVolt_A(QByteArray DataPack)
{
    double sipmvoltege_A = 0.0;
    int sipmvoltege_A_adc = 256 * DataPack[23-2] + DataPack[24-2];
    //sipmvoltege_A = (sipmvoltege_A_adc * 3.3) / 4096;
    //sipmvoltege_A = sipmvoltege_A * 105 / 5;  //需要注意，这里的分压系数还需要实验来进行确定
    sipmvoltege_A = sipmvoltege_A_adc * 0.020147;
    return sipmvoltege_A;
}

// 获取探测器B组偏压
// DataPack是不包含包头的数据包,包头2字节
double mainWindow::GetVolt_B(QByteArray DataPack)
{
    double sipmvoltege_B = 0.0;
    int sipmvoltege_B_adc = 256 * DataPack[25-2] + DataPack[26-2];
    //sipmvoltege_B = (sipmvoltege_B_adc * 3.3) / 4096;
    // sipmvoltege_B = sipmvoltege_B * 105 / 5; //需要注意，这里的分压系数还需要实验来进行确定
    sipmvoltege_B = sipmvoltege_B_adc * 0.017284;
    return sipmvoltege_B;
}

//开始测量&停止测量按钮
void mainWindow::on_Measure_Button_clicked() 
{
    if (ui->Measure_Button->text() == "开始测量")
    {
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
        MeasureStaus = true;
        refreshPlotFlag = true;
        //-----------控件-----------
        ui->refreshPlotCheckBox->setCheckState(Qt::Checked); // 图像刷新
        ui->experimentNameEdit->setEnabled(false);//禁止输入状态

        // =========PC端向ARM端发送开始测量指令==============
        tcpSocket->write(tcp_order.StartMeasure); WaitingSocketWrite(); Sleep(tcp_order.waitingTime);
        
        // ==============记录实验开始时间==================
        beginTime = QDateTime::currentDateTime();
        QString str_beginTime = beginTime.toString("yyyy-MM-dd hh:mm:ss");
        ui->measureTime_label->setText(str_beginTime);

        //===============②生成默认路径字符串====================
        QString EquipmentID = ui->equipmentID_label->text();
        experimentName = ui->experimentNameEdit->toPlainText();
        QString Filepath = QCoreApplication::applicationDirPath(); // 获取当前exe文件所在路径
        QString dirName = Filepath + "/" + "GMCOUNTER";
        autofilePath = dirName + "/" +  + "设备" + EquipmentID + "_" + experimentName 
            + beginTime.toString("_yyyy-MM-dd_hh-mm-ss") + ".txt";

        // 开启定时器绘图
        /*
        timer = new QTimer();
        timer->setInterval(1000); //单位：ms
        timer->start();
        connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut())); // 根据定时器触发绘图
        */
        
        ui->Measure_Button->setText(QString("停止测量"));
    }
    else if (ui->Measure_Button->text() == "停止测量")
    {
        /*
        //定时器关闭，停止绘图
        timer->stop(); 
        delete timer;
        */
        MeasureStaus = false;
        // PC端向ARM端发送停止测量指令
        tcpSocket->write(tcp_order.StopMeasure); WaitingSocketWrite(); Sleep(tcp_order.waitingTime);
        
        // 保存测量数据
        if (counter1.size() > 0) {
            SaveFile(autofilePath, counter1, counter2, counter3, counter4, temperatue);//保存这次的测量数据至默认路径
        }
        
        // 另存为，弹出是否保存文件的对话框
        if (counter1.size() > 0) {
            QJsonObject jsonSetting = ReadSetting();
            QString saveDir = jsonSetting["SaveDir"].toString(); //"SaveDir": "/home"
            // 判断是否存在默认路径，不存在则读取桌面所在目录。
            QDir dir(saveDir);
            if (!dir.exists()) {
                QStringList desktopLocation = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);//如果不存在该目录，则重定向到桌面所在目录
                saveDir = desktopLocation.at(0);
            }

            QString fileFullName  = QFileDialog::getSaveFileName(this, tr("保存文件"),
                                                                saveDir,
                                                                tr("TXT files(*.txt)"));
            // 判断文件名是否获取到
            if (!fileFullName.isEmpty()){
                QFileInfo fileinfo = QFileInfo(fileFullName);
                //QString file_name = fileinfo.fileName();//文件名称
                //QString file_suffix = fileinfo.suffix();//文件后缀格式
                QString file_path = fileinfo.absolutePath();//文件绝对路径

                jsonSetting["SaveDir"] = file_path;
                WriteSetting(jsonSetting);
                SaveFile(fileFullName, counter1, counter2, counter3, counter4, temperatue, timeLength);
            } 
        }
        // 恢复使用
        ui->Measure_Button->setText(QString("开始测量")); //按钮翻转
        ui->experimentNameEdit->setEnabled(true);//恢复输入状态
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

    // 自动调节坐标轴
    if (RescaleAxesFlag){
        pGraph1_1->rescaleAxes(); // 让范围自行缩放，使图0完全适合于可见区域.这里不能带参数true
        pGraph1_2->rescaleAxes(true); // 图1也是一样自动调整范围，但只是放大或不变范围
        pGraph1_3->rescaleAxes(true);
        pGraph1_4->rescaleAxes(true);
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
    for (int i = pPlot->graphCount() - 1; i >= 0; --i)
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
            tip = QString::number(i+1) + "," + QString::number(x_value) + "," + QString::number(y_value);
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
    qDebug() << "Text:" << str;
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
        ////设置追踪曲线
        //for (int i = 0; i < 4; i++) {
        //    plottracer[i] = new QCPItemTracer(pPlot);
        //    plottracer[i]->setGraph(pPlot->graph(i));
        //    //设置十字浮标样式
        //    QPen pen = pPlot->graph(i)->pen();
        //    pen.setStyle(Qt::SolidLine);//
        //    plottracer[i]->setPen(pen);
        //}
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
        pGraph1_2->rescaleAxes(true); // 图1也是一样自动调整范围，但只是放大或不变范围
        pGraph1_3->rescaleAxes(true);
        pGraph1_4->rescaleAxes(true);
    }
    else { //未选中
        RescaleAxesFlag = false;
        pGraph1_1->rescaleAxes(false); 
        pGraph1_2->rescaleAxes(false); 
        pGraph1_3->rescaleAxes(false);
        pGraph1_4->rescaleAxes(false);
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
    // 关闭前处于测量状态，PC端向ARM端发送停止测量指令
    if (ui->Measure_Button->text() == "停止测量")
    {
        tcpSocket->write(tcp_order.StopMeasure);  WaitingSocketWrite();
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
    if (tcpSocket) {
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
