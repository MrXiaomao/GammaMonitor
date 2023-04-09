#pragma once

#include <QDialog>
#include "ui_RelayDialog.h"
#include <QTcpSocket>
#include "order.h"

// ���Ƽ̵����Ĵ���
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
	void on_connectRelayButton_clicked(); // ���Ƽ̵�������������
	void on_controlRelayButton_clicked(); // ���Ƽ̵�������
	void onTimeOut(); //��ʱ��

private:
	Ui::RelayDialog ui;

	void WaitingSocketWrite(int time = 30000); // �ȴ�QTcpSocketд������
	QTcpSocket* tcpSocket;// ֱ�ӽ���TCP�׽�����
	QString tcpIp;// �洢IP��ַ
	QString tcpPort;// �洢�˿ڵ�ַ
	Order tcp_order; // PC�˷��͵�ָ��
	QTimer* timer; //��ʱ��
};
