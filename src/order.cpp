#include "order.h"
Order::Order(void)
{
	waitingTime = 20; //��������д��ȴ�ʱ�䣬�ȴ�ARM��Ӧָ�

	// ==========================���ֽ�ָ��=====================
	//
	StartMeasure.resize(4);
	StopMeasure.resize(4);

	DetecA_ON.resize(4);
	DetecA_ON.resize(4);
	ExtDeviceON.resize(4);

	DetecA_OFF.resize(4);
	DetecB_OFF.resize(4);
	ExtDeviceOFF.resize(4);

	//��ʼ����
	//04 AA 00 00
	StartMeasure[0] = 0x04;
	StartMeasure[1] = 0xAA;
	StartMeasure[2] = 0x00;
	StartMeasure[3] = 0x00;

	//ֹͣ����
	//04 BB 00 00
	StopMeasure[0] = 0x04;
	StopMeasure[1] = 0xBB;
	StopMeasure[2] = 0x00;
	StopMeasure[3] = 0x00;

	//����A��̽����SiPMƫѹ
	//60 00 00 00
	DetecA_ON[0] = 0x60;
	DetecA_ON[1] = 0x00;
	DetecA_ON[2] = 0x00;
	DetecA_ON[3] = 0x00;

	//����A��̽����SiPMƫѹ
	//60 01 00 00
	DetecB_ON[0] = 0x60;
	DetecB_ON[1] = 0x01;
	DetecB_ON[2] = 0x00;
	DetecB_ON[3] = 0x00;

	//���������ѹ
	//60 02 00 00
	ExtDeviceON[0] = 0x60;
	ExtDeviceON[1] = 0x02;
	ExtDeviceON[2] = 0x00;
	ExtDeviceON[3] = 0x00;

	//�ر�A��̽����SiPMƫѹ
	//40 00 00 00
	DetecA_OFF[0] = 0x40;
	DetecA_OFF[1] = 0x00;
	DetecA_OFF[2] = 0x00;
	DetecA_OFF[3] = 0x00;

	//�ر�B��̽����SiPMƫѹ
	//40 01 00 00
	DetecB_OFF[0] = 0x40;
	DetecB_OFF[1] = 0x01;
	DetecB_OFF[2] = 0x00;
	DetecB_OFF[3] = 0x00;

	//�ر������ѹ
	//40 02 00 00
	ExtDeviceOFF[0] = 0x40;
	ExtDeviceOFF[1] = 0x02;
	ExtDeviceOFF[2] = 0x00;
	ExtDeviceOFF[3] = 0x00;

	// =================================���ֽ�ָ��=======================================
	//
	MonitorMessageON.resize(6);
	MonitorMessageOFF.resize(6);

	DetectorThread.resize(6);
	DetectorThreadOFF.resize(6);

	VoltageA_MonitorON.resize(6);
	VoltageB_MonitorON.resize(6);
	Temp_MonitorON.resize(6);
	InputVoltage_MonitorON.resize(6);

	VoltageA_MonitorOFF.resize(6);
	VoltageB_MonitorOFF.resize(6);
	Temp_MonitorOFF.resize(6);
	InputVoltage_MonitorOFF.resize(6);

	// ��ARM���������豸״̬��Ϣ���¶ȡ������Դ��̽����A���ѹ��̽����B���ѹ��
	//80 AA 00 00 00 00
	MonitorMessageON[0] = 0x80;
	MonitorMessageON[1] = 0xAA;
	MonitorMessageON[2] = 0x00;
	MonitorMessageON[3] = 0x00;
	MonitorMessageON[4] = 0x00;
	MonitorMessageON[5] = 0x00;

	// ��ARMֹͣ�����豸״̬��Ϣ���¶ȡ������Դ��̽����A���ѹ��̽����B���ѹ��
	// 90 AA 00 00 00 00
	MonitorMessageOFF[0] = 0x90;
	MonitorMessageOFF[1] = 0xAA;
	MonitorMessageOFF[2] = 0x00;
	MonitorMessageOFF[3] = 0x00;
	MonitorMessageOFF[4] = 0x00;
	MonitorMessageOFF[5] = 0x00;

	// �Ƚ�����ֵĬ��ֵ,��������̽�����Ĵ�����ֵΪ 42����0.034V
	//50 01 00 2A 00 2A
	DetectorThread[0] = 0x50;
	DetectorThread[1] = 0x01;
	DetectorThread[2] = 0x00;
	DetectorThread[3] = 0x2A;
	DetectorThread[4] = 0x00;
	DetectorThread[5] = 0x2A;

	// �ر�����̽�����ıȽ���
	DetectorThreadOFF[0] = 0x50;
	DetectorThreadOFF[1] = 0x00;
	DetectorThreadOFF[2] = 0x00;
	DetectorThreadOFF[3] = 0x00;
	DetectorThreadOFF[4] = 0x00;
	DetectorThreadOFF[5] = 0x00;

	// ����A��ƫѹ���
	//80 01 00 00 00 00
	VoltageA_MonitorON[0] = 0x80;
	VoltageA_MonitorON[1] = 0x01;
	VoltageA_MonitorON[2] = 0x00;
	VoltageA_MonitorON[3] = 0x00;
	VoltageA_MonitorON[4] = 0x00;
	VoltageA_MonitorON[5] = 0x00;

	// ����B��ƫѹ���
	// 80 02 00 00 00 00
	VoltageB_MonitorON[0] = 0x80;
	VoltageB_MonitorON[1] = 0x02;
	VoltageB_MonitorON[2] = 0x00;
	VoltageB_MonitorON[3] = 0x00;
	VoltageB_MonitorON[4] = 0x00;
	VoltageB_MonitorON[5] = 0x00;

	// �����¶ȼ�� 
	// 80 03 00 00 00 00
	Temp_MonitorON[0] = 0x80;
	Temp_MonitorON[1] = 0x03;
	Temp_MonitorON[2] = 0x00;
	Temp_MonitorON[3] = 0x00;
	Temp_MonitorON[4] = 0x00;
	Temp_MonitorON[5] = 0x00;

	// �����豸�����ѹ���
	// 80 04 00 00 00 00
	InputVoltage_MonitorON[0] = 0x80;
	InputVoltage_MonitorON[1] = 0x04;
	InputVoltage_MonitorON[2] = 0x00;
	InputVoltage_MonitorON[3] = 0x00;
	InputVoltage_MonitorON[4] = 0x00;
	InputVoltage_MonitorON[5] = 0x00;	

	// �ر�̽������Aƫѹ���
	// 90 01 00 00 00 00
	VoltageA_MonitorOFF[0] = 0x90;
	VoltageA_MonitorOFF[1] = 0x01;
	VoltageA_MonitorOFF[2] = 0x00;
	VoltageA_MonitorOFF[3] = 0x00;
	VoltageA_MonitorOFF[4] = 0x00;
	VoltageA_MonitorOFF[5] = 0x00;

	// �ر�̽������Bƫѹ���
	// 90 02 00 00 00 00
	VoltageB_MonitorOFF[0] = 0x90;
	VoltageB_MonitorOFF[1] = 0x02;
	VoltageB_MonitorOFF[2] = 0x00;
	VoltageB_MonitorOFF[3] = 0x00;
	VoltageB_MonitorOFF[4] = 0x00;
	VoltageB_MonitorOFF[5] = 0x00;

	// �ر��¶ȼ��
	// 90 03 00 00 00 00
	Temp_MonitorOFF[0] = 0x90;
	Temp_MonitorOFF[1] = 0x03;
	Temp_MonitorOFF[2] = 0x00;
	Temp_MonitorOFF[3] = 0x00;
	Temp_MonitorOFF[4] = 0x00;
	Temp_MonitorOFF[5] = 0x00;

	// �ر�5V��ѹ���
	// 90 04 00 00 00 00
	InputVoltage_MonitorOFF[0] = 0x90;
	InputVoltage_MonitorOFF[1] = 0x04;
	InputVoltage_MonitorOFF[2] = 0x00;
	InputVoltage_MonitorOFF[3] = 0x00;
	InputVoltage_MonitorOFF[4] = 0x00;
	InputVoltage_MonitorOFF[5] = 0x00;

	QByteArray VoltageB_MonitorOFF; // �ر�̽������Bƫѹ���
	QByteArray Temp_MonitorOFF; // �ر��¶ȼ��
	QByteArray InputVoltage_MonitorOFF; // �ر��豸�����ѹ��⣨5V��
	//�̵������ƣ�����ASCII�ı�������ʮ������
	PowerCH1_ON = "21\0";
	PowerCH1_OFF = "11\0";
	PowerCH2_ON = "22\0";
	PowerCH2_OFF = "12\0";
	PowerStatus = "00\0";
}


Order::~Order(void)
{}
