// 功能：①存放PC端对ARM硬件发送的指令；
//       ②存放继电器指令
// 
// 开发人员：毛潇庆 1207402640@qq.com
// 开发时间：2023年2月18日20:16:42
#pragma once
#include "QByteArray"
class Order
{
public:
	Order(void);
	~Order(void);

	int waitingTime; //网口数据写入等待时间，等待ARM响应指令。
	// ===================四字节指令===================
	QByteArray StartMeasure; // 开始计数
	QByteArray StopMeasure; // 停止计数

	QByteArray DetecA_ON; // 开启探测器组A偏压
	QByteArray DetecB_ON; // 开启探测器组B偏压
	QByteArray ExtDeviceON; // 开启外设供电

	QByteArray DetecA_OFF; // 关闭探测器组A偏压
	QByteArray DetecB_OFF; // 关闭探测器组B偏压
	QByteArray ExtDeviceOFF; // 关闭外接设备供电

	// ===================六字节指令===================
	QByteArray MonitorMessageON; // 让ARM开始发送设备状态信息（也就是温度、输入电源、探测器A组电压、探测器B组电压四种数据）
	QByteArray MonitorMessageOFF; // 让ARM停止发送设备状态信息

	QByteArray DetectorThread; // 开启并设置两组探测器的比较器阈值
	QByteArray DetectorThreadOFF; // 关闭两组探测器的比较器

	QByteArray VoltageA_MonitorON; // 开启探测器组A偏压监测
	QByteArray VoltageB_MonitorON; // 开启探测器组B偏压监测
	QByteArray Temp_MonitorON; // 开启温度监测
	QByteArray InputVoltage_MonitorON; // 开启设备输入电压监测（5V）

	QByteArray VoltageA_MonitorOFF; // 关闭探测器组A偏压监测
	QByteArray VoltageB_MonitorOFF; // 关闭探测器组B偏压监测
	QByteArray Temp_MonitorOFF; // 关闭温度监测
	QByteArray InputVoltage_MonitorOFF; // 关闭设备输入电压监测（5V）

	// ===================继电器指令=================
	char* PowerCH1_ON; // 继电器通道1吸合（通道1对应正极）
	char* PowerCH1_OFF; // 继电器通道1释放
	char* PowerCH2_ON; // 继电器通道2吸合（通道2对应负极/地线）
	char* PowerCH2_OFF; // 继电器通道2释放
	char* PowerStatus; // 查询继电器吸合状态

private:
};

