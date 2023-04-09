
#pragma once

#include <QDialog>
#include "ui_ResetNetwork.h"
#include <QTcpSocket>
#include "order.h"

// ���Ƽ̵����Ĵ���
class ResetNetwork : public QDialog
{
	Q_OBJECT

public:
	ResetNetwork(QWidget* parent = Q_NULLPTR);
	~ResetNetwork();
	//static quint32 IPV4StringToInteger(const QString& ip);

private slots:
	void readMassage();
	void displayError(QAbstractSocket::SocketError);
	void connectUpdata();
	void disconnectUpdata();
	void on_connectButton_clicked(); // ���Ƽ̵�������������
	void on_changeSetting_clicked(); // �޸�����
	void onTimeOut(); //��ʱ��
	void closeEvent(QCloseEvent* event);

private:
	Ui::ResetNetwork ui;

	void WaitingSocketWrite(int time = 30000); // �ȴ�QTcpSocketд������
	QTcpSocket* tcpSocket;// ֱ�ӽ���TCP�׽�����
	QString tcpIp;// �洢IP��ַ
	QString tcpGateway; // �洢����
	QString tcpPort;// �洢�˿ڵ�ַ
	Order tcp_order; // PC�˷��͵�ָ��
	QTimer* timer; //��ʱ��
};
