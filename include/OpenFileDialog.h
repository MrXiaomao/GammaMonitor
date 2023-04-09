#pragma once

#include <QDialog>
#include "ui_OpenFileDialog.h"
#include "mytracer.h"
#include "mytracerline.h"
#include "mainWindow.h"

//enum class TracerFlag
//{
//	CrossTracer, //十字光标追踪
//	CurveTracer, //曲线追踪 
//	NoTracer     //无追踪
//};
enum KeyboardType {
	leftMove, // 键盘左移
	rightMove // 键盘右移
};

// 读取文件，绘制曲线
class OpenFileDialog : public QDialog
{
	Q_OBJECT

public:
	OpenFileDialog(QString fileName, QWidget *parent = Q_NULLPTR);
	~OpenFileDialog();
	void ReadFile(); // 读取文件
	void QPlot_init(QCustomPlot* customPlot);// 画布初始化
	void ShowPlot(); // 绘图，绘制4条曲线
	void DoCrossTracer(QMouseEvent* event); // 十字架取值
	void DoCurveTracer(QMouseEvent* event); // 曲线取值
	void DoCurveTracer(KeyboardType type); // 曲线取值,响应键盘左右键移动

	void UpdateAxisRange(); // 刷新坐标轴范围的显示值与图像对应
	void setAxisRange(); // 根据界面输入框设置坐标轴范围
	void saveFilePath(QString fileFullName);

private:
	Ui::OpenFileDialog ui;
	QString myFileName;
	QVector<double> counter1; //存放四个探测器的计数。
	QVector<double> counter2;
	QVector<double> counter3;
	QVector<double> counter4;

	// 绘图控件的指针
	QCustomPlot* pPlot;

	// 绘图控件中曲线的指针
	QCPGraph* pGraph1;
	QCPGraph* pGraph2;
	QCPGraph* pGraph3;
	QCPGraph* pGraph4;

	// 曲线光标
	TracerFlag mTracer;
	myTracer* tracerCross; // 十字光标
	myTracer* tracerX[4];
	myTracerLine* lineTracer; // 直线光标
	int lastPos; // 记录光标上一次的值

public slots:
	bool eventFilter(QObject* watched, QEvent* event);
	void DoubleClick(); //鼠标双击还原
	void myMoveMouse(); // 鼠标拖动

private slots:
	bool on_pbn_save_clicked(); // 导出图像
	void on_GetData_comboBox_currentIndexChanged(const QString& arg1); //是否光标，取值类型
	void SLOT_mouseTracetoCoord(QMouseEvent* event); //鼠标按下触发的槽函数
};
