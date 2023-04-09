#pragma once

#include <QDialog>
#include "ui_RelayDialog.h"
#include <QTcpSocket>
#include "order.h"

// 控制继电器的窗口
class RelayDialog : public QDialog
{
	Q_OBJECT

public:
	RelayDialog(QWidget *parent = Q_NULLPTR);
	~RelayDialog();

private slots:	
	void readMassage();
	void displayError(QAbstractSocket::SocketError);
	void connectUpdata();
	void disconnectUpdata();
	void on_connectRelayButton_clicked(); // 控制继电器的网络连接
	void on_controlRelayButton_clicked(); // 控制继电器开关
	void onTimeOut(); //定时器

private:
	Ui::RelayDialog ui;

	void WaitingSocketWrite(int time = 30000); // 等待QTcpSocket写入数据
	QTcpSocket* tcpSocket;// 直接建立TCP套接字类
	QString tcpIp;// 存储IP地址
	QString tcpPort;// 存储端口地址
	Order tcp_order; // PC端发送的指令
	QTimer* timer; //定时器
};
