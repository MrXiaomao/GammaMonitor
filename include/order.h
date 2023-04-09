// ���ܣ��ٴ��PC�˶�ARMӲ�����͵�ָ�
//       �ڴ�ż̵���ָ��
// 
// ������Ա��ë���� 1207402640@qq.com
// ����ʱ�䣺2023��2��18��20:16:42
#pragma once
#include "QByteArray"
class Order
{
public:
	Order(void);
	~Order(void);

	int waitingTime; //��������д��ȴ�ʱ�䣬�ȴ�ARM��Ӧָ�
	// ===================���ֽ�ָ��===================
	QByteArray StartMeasure; // ��ʼ����
	QByteArray StopMeasure; // ֹͣ����

	QByteArray DetecA_ON; // ����̽������Aƫѹ
	QByteArray DetecB_ON; // ����̽������Bƫѹ
	QByteArray ExtDeviceON; // �������蹩��

	QByteArray DetecA_OFF; // �ر�̽������Aƫѹ
	QByteArray DetecB_OFF; // �ر�̽������Bƫѹ
	QByteArray ExtDeviceOFF; // �ر�����豸����

	// ===================���ֽ�ָ��===================
	QByteArray MonitorMessageON; // ��ARM��ʼ�����豸״̬��Ϣ��Ҳ�����¶ȡ������Դ��̽����A���ѹ��̽����B���ѹ�������ݣ�
	QByteArray MonitorMessageOFF; // ��ARMֹͣ�����豸״̬��Ϣ

	QByteArray DetectorThread; // ��������������̽�����ıȽ�����ֵ
	QByteArray DetectorThreadOFF; // �ر�����̽�����ıȽ���

	QByteArray VoltageA_MonitorON; // ����̽������Aƫѹ���
	QByteArray VoltageB_MonitorON; // ����̽������Bƫѹ���
	QByteArray Temp_MonitorON; // �����¶ȼ��
	QByteArray InputVoltage_MonitorON; // �����豸�����ѹ��⣨5V��

	QByteArray VoltageA_MonitorOFF; // �ر�̽������Aƫѹ���
	QByteArray VoltageB_MonitorOFF; // �ر�̽������Bƫѹ���
	QByteArray Temp_MonitorOFF; // �ر��¶ȼ��
	QByteArray InputVoltage_MonitorOFF; // �ر��豸�����ѹ��⣨5V��

	// ===================�̵���ָ��=================
	char* PowerCH1_ON; // �̵���ͨ��1���ϣ�ͨ��1��Ӧ������
	char* PowerCH1_OFF; // �̵���ͨ��1�ͷ�
	char* PowerCH2_ON; // �̵���ͨ��2���ϣ�ͨ��2��Ӧ����/���ߣ�
	char* PowerCH2_OFF; // �̵���ͨ��2�ͷ�
	char* PowerStatus; // ��ѯ�̵�������״̬

private:
};

