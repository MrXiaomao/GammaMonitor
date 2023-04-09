#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_mainWindow.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QJsonObject> 
#include "order.h"
#include "CustomPlotTooltip.h"
#include "mytracer.h"
#include "mytracerline.h"

enum class TracerFlag
{
    CrossTracer, //十字光标追踪
    CurveTracer, //曲线追踪 
    NoTracer     //无追踪
};

class mainWindow : public QMainWindow
{
    Q_OBJECT

public:

    mainWindow(QWidget *parent = Q_NULLPTR);
    ~mainWindow();//析构函数

    static QJsonObject ReadSetting(); // 读取配置文件
    static void WriteSetting(QJsonObject);  // 写配置文件
    void closeAction(); // 响应关闭窗口动作，对一些数据进行处理，以及arm进行最后通信
    void DoCrossTracer(QMouseEvent* event); // 十字架取值
    void DoCurveTracer(QMouseEvent* event); // 曲线取值

private slots:
    void readMassage(); // 读取网口数据
    void displayError(QAbstractSocket::SocketError);
    void connectUpdata(); // 连接成功，更新各个按钮功能
    void disconnectUpdata(); // 断开连接，更新各个按钮功能
    void on_connectButton_clicked(); // 网络连接按钮
    void on_Measure_Button_clicked(); // 开始测量&停止测量按钮
    void on_networkSettingMenu_triggered(); // 菜单栏网络设置
    void on_relayMenu_triggered(); // 菜单栏继电器控制
    void on_setThresholdMenu_triggered(); // 探测器阈值设置
    void on_openFileMenu_triggered(); //打开历史文件
    void on_checkBox1_stateChanged(int arg1); //复选框1
    void on_checkBox2_stateChanged(int arg1); //复选框2
    void on_checkBox3_stateChanged(int arg1); //复选框3
    void on_checkBox4_stateChanged(int arg1); //复选框4
    void on_GetData_comboBox_currentIndexChanged(const QString& arg1); //是否光标，取值类型
    void on_refreshPlotCheckBox_stateChanged(int arg1); //响应图像刷新
    void on_rescaleAxesCheckBox_stateChanged(int arg1); //坐标轴自适应
    
    void SLOT_mouseTracetoCoord(QMouseEvent* event);//鼠标按下触发的槽函数

protected slots:
    //void onTimeOut();
    void closeEvent(QCloseEvent* event);

private:
    Ui::mainWindowClass *ui;

    void PlotData(const QVector<double>& x,const QVector<double>& y, QColor color = Qt::red); // 对输入的一对数据进行绘图
    // 保存测量数据
    void SaveFile(QString filepath, QVector<int>data1, QVector<int>data2,
                  QVector<int>data3, QVector<int>data4, QVector<double>temp, int timeLen = 0);

    QTcpSocket* tcpSocket;//直接建立TCP套接字类
    QString tcpIp;// 存储IP地址
    QString tcpPort;// 存储端口地址
    Order tcp_order; // PC端发送的指令

    int PackNumber; // 记录数据包的数目
    int EquipmentID; // 设备编号
    QByteArray TotalPackArray; // 接受的来自网口的数据
    QByteArray dataOnePack; // 单个包的数据

    QString experimentName; // 实验名称
    QString autofilePath; // 自动备份的文件路径

    QVector<int> counter1; //存放四个探测器的计数。
    QVector<int> counter2;
    QVector<int> counter3;
    QVector<int> counter4;
    QVector<double> temperatue; // 记录测量时刻对应温度

    QTimer* timer; //定时器
    QDateTime beginTime; //测量开始时间
    QDateTime nowTime; //当前时间
    int timeLength; //测量时长，单位：s
    bool MeasureStaus; //测量状态标志


    // 绘图控件的指针
    QCustomPlot* pPlot1;
    double plotCount ; //记录绘图的数据点个数，注意由于会定时矫正ARM的数据包个数，因此PackNumber与plotCount不一定相等。 
    QCPToolTip* m_pCpTip;

    // 状态栏指针
    QStatusBar* sBar;
    // 绘图控件中曲线的指针
    QCPGraph* pGraph1_1;
    QCPGraph* pGraph1_2;
    QCPGraph* pGraph1_3;
    QCPGraph* pGraph1_4;

    TracerFlag mTracer;
    myTracer* tracerCross;
    myTracer* tracerX[4];
    myTracerLine* lineTracer; // 直线

    bool refreshPlotFlag; // 是否刷新图像
    bool RescaleAxesFlag; // 是否自动调整坐标轴范围

    void QPlot_init(QCustomPlot* customPlot);
    void Show_Plot(QCustomPlot* customPlot, double num1,double num2,double num3, double num4);

    void ARM_Sleep(); // 让ARM进入休眠，停止比较器工作，停止电压监测、温度监测
    void WaitingSocketWrite(int time = 30000); // 等待QTcpSocket写入数据
    void GetCounter(QByteArray DataPack,int *count); // 解析四个探测器计数
    double GetTemperature(QByteArray DataPack); // 解析数据包中的温度
    double GetOuterVolt(QByteArray DataPack);// 获取外部电压
    double GetVolt_A(QByteArray DataPack);// 获取探测器A组偏压
    double GetVolt_B(QByteArray DataPack);// 获取探测器B组偏压

    QCPItemTracer* plottracer[4]; //定义一个鼠标追踪变量
};
