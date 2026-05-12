/*
 * @Author: MrPan
 * @Date: 2026-05-10 19:34:32
 * @LastEditors: Maoxiaoqing
 * @LastEditTime: 2026-05-12 13:52:40
 * @Description: 请填写简介
 */

#pragma once

#include <QDialog>
#include "ui_ResetNetwork.h"
#include <QTcpSocket>
#include "order.h"

// 控制继电器的窗口
class ResetNetwork : public QDialog
{
	Q_OBJECT

public:
	ResetNetwork(QWidget* parent = Q_NULLPTR);
	~ResetNetwork();

private slots:
	void readMassage();
	void displayError(QAbstractSocket::SocketError);
	void connectUpdata();
	void disconnectUpdata();
	void on_bt_connectDet_clicked(); // 控制探测器的网络连接
	void on_changeSetting_clicked(); // 修改配置
	void closeEvent(QCloseEvent* event);

private:
	Ui::ResetNetwork ui;

	void WaitingSocketWrite(int time = 30000); // 等待QTcpSocket写入数据
	QTcpSocket* tcpSocket;// 直接建立TCP套接字类
	QString tcpIp;// 存储IP地址
	QString tcpGateway; // 存储网关
	QString tcpPort;// 存储端口地址
	Order tcp_order; // PC端发送的指令
};
