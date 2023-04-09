#pragma once

#include <QDialog>
#include "ui_OpenFileDialog.h"
#include "mytracer.h"
#include "mytracerline.h"
#include "mainWindow.h"

//enum class TracerFlag
//{
//	CrossTracer, //ʮ�ֹ��׷��
//	CurveTracer, //����׷�� 
//	NoTracer     //��׷��
//};
enum KeyboardType {
	leftMove, // ��������
	rightMove // ��������
};

// ��ȡ�ļ�����������
class OpenFileDialog : public QDialog
{
	Q_OBJECT

public:
	OpenFileDialog(QString fileName, QWidget *parent = Q_NULLPTR);
	~OpenFileDialog();
	void ReadFile(); // ��ȡ�ļ�
	void QPlot_init(QCustomPlot* customPlot);// ������ʼ��
	void ShowPlot(); // ��ͼ������4������
	void DoCrossTracer(QMouseEvent* event); // ʮ�ּ�ȡֵ
	void DoCurveTracer(QMouseEvent* event); // ����ȡֵ
	void DoCurveTracer(KeyboardType type); // ����ȡֵ,��Ӧ�������Ҽ��ƶ�

	void UpdateAxisRange(); // ˢ�������᷶Χ����ʾֵ��ͼ���Ӧ
	void setAxisRange(); // ���ݽ�����������������᷶Χ
	void saveFilePath(QString fileFullName);

private:
	Ui::OpenFileDialog ui;
	QString myFileName;
	QVector<double> counter1; //����ĸ�̽�����ļ�����
	QVector<double> counter2;
	QVector<double> counter3;
	QVector<double> counter4;

	// ��ͼ�ؼ���ָ��
	QCustomPlot* pPlot;

	// ��ͼ�ؼ������ߵ�ָ��
	QCPGraph* pGraph1;
	QCPGraph* pGraph2;
	QCPGraph* pGraph3;
	QCPGraph* pGraph4;

	// ���߹��
	TracerFlag mTracer;
	myTracer* tracerCross; // ʮ�ֹ��
	myTracer* tracerX[4];
	myTracerLine* lineTracer; // ֱ�߹��
	int lastPos; // ��¼�����һ�ε�ֵ

public slots:
	bool eventFilter(QObject* watched, QEvent* event);
	void DoubleClick(); //���˫����ԭ
	void myMoveMouse(); // ����϶�

private slots:
	bool on_pbn_save_clicked(); // ����ͼ��
	void on_GetData_comboBox_currentIndexChanged(const QString& arg1); //�Ƿ��꣬ȡֵ����
	void SLOT_mouseTracetoCoord(QMouseEvent* event); //��갴�´����Ĳۺ���
};
