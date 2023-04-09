#include "SetThreshlod.h"
#include "order.h"
#include "mainWindow.h"
#include <qvalidator.h>

SetThreshlod::SetThreshlod(QTcpSocket* tcpSocket, QWidget *parent)
	: parentSocket(tcpSocket),QDialog(parent)
{
	ui.setupUi(this);
	ui.thresholdA_Edit->setValidator(new QIntValidator(0, 4095, this));  // �˿ں�ֻ����[0,4095]��Χ�ڵ���������
	ui.thresholdB_Edit->setValidator(new QIntValidator(0, 4095, this));  // �˿ں�ֻ����[0,4095]��Χ�ڵ���������
	QJsonObject jsonSetting = mainWindow::ReadSetting();
	QString strT1 = jsonSetting["ThresholdA"].toString();
	QString strT2 = jsonSetting["ThresholdB"].toString();
	ui.thresholdA_Edit->setText(strT1);
	ui.thresholdB_Edit->setText(strT2);
}

SetThreshlod::~SetThreshlod()
{
}

void SetThreshlod::on_SetButton_clicked() {
	QString thresholdA = ui.thresholdA_Edit->text();
	QString thresholdB = ui.thresholdA_Edit->text();
	// �������ݵ������ļ�
	QJsonObject jsonSetting = mainWindow::ReadSetting();
	jsonSetting["ThresholdA"] = thresholdA;
	jsonSetting["ThresholdB"] = thresholdB;
	mainWindow::WriteSetting(jsonSetting);

	int T1 = thresholdA.toInt();
	int T2 = thresholdB.toInt();
	if (T1 < 0 && T1 > 4095) {
		QMessageBox::warning(this, "������Ϣ", "��������ȷ����ֵ��0-4095��");
		return;
	}
	if (T2 < 0 && T2 > 4095) {
		QMessageBox::warning(this, "������Ϣ", "��������ȷ����ֵ��0-4095��");
		return;
	}

	QByteArray msg;
	msg.resize(6);
	msg[0] = 0x50;
	msg[1] = 0x01;
	msg[2] = T1 / 256;
	msg[3] = T1 % 256;
	msg[4] = T2 / 256;
	msg[5] = T2 % 256;
	parentSocket->write(msg);
	QMessageBox::about(this, "��ʾ��Ϣ", "��ֵ���óɹ�");
}