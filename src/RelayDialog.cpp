#include "RelayDialog.h"
#include "mainWindow.h"

#include <QElapsedTimer>

#pragma execution_character_set("utf-8") 

// ���Ƽ̵����Ĵ���
RelayDialog::RelayDialog(QWidget *parent)
	: QDialog(parent)
{
	ui.setupUi(this);
	// ��ȡ�����ļ���json�ļ�
	QJsonObject jsonSetting = mainWindow::ReadSetting();
    tcpIp = jsonSetting["IP_Relay"].toString();
    tcpPort = jsonSetting["Port_Relay"].toString();
    
    ui.controlRelayButton->setEnabled(false);// ���������ؽ�ֹ״̬
    ui.IP_RelayEdit->setText(tcpIp);
    ui.Port_RelayEdit->setText(tcpPort);
    ui.Port_RelayEdit->setValidator(new QIntValidator(1, 9999, this));  // �˿ں�ֻ����[1,9999]��Χ�ڵ���������
    timer = Q_NULLPTR;
    tcpSocket = Q_NULLPTR;//ʹ��ǰ����� 
}

RelayDialog::~RelayDialog()
{
    //��ʱ���ر�
    if (timer) {
        if (timer->isActive())//�ж϶�ʱ���Ƿ��ڹ���
            timer->stop();
        delete timer;
    }
    if(tcpSocket) delete tcpSocket;
}

void RelayDialog::on_connectRelayButton_clicked()
{
    if (ui.connectRelayButton->text() == "����")
    {
        ui.connectRelayButton->setEnabled(false); // ���Ӱ�ť��ֹʹ�ã���ϵͳ��Ӧ����
        ui.NetStatusLabel->setText("���������С�����");

        if (tcpSocket) delete tcpSocket; //�����ָ�������ռ�ֱ��ɾ��
        tcpSocket = new QTcpSocket(this); //����ѿռ���TCP���ͺͽ��ܲ���

        // ������Ӱ�ť�󣬼�¼�µ�ǰIP�Լ�Port
        tcpIp = ui.IP_RelayEdit->text();
        tcpPort = ui.Port_RelayEdit->text();
        QJsonObject jsonSetting = mainWindow::ReadSetting();
        jsonSetting["IP_Relay"] = tcpIp;
        jsonSetting["Port_Relay"] = tcpPort;
        mainWindow::WriteSetting(jsonSetting);

        if(!ui.IP_RelayEdit->isTextValid(tcpIp))
        {
            QMessageBox msgBox;
            msgBox.setWindowTitle("Warning");
            msgBox.setText("IP is in valid");
            msgBox.exec();
            return;
        }

        tcpSocket->connectToHost(tcpIp, tcpPort.toInt());//��������
        connect(tcpSocket, SIGNAL(error(QAbstractSocket::SocketError)), this,
            SLOT(displayError(QAbstractSocket::SocketError)));//��������
        connect(tcpSocket, SIGNAL(connected()), this, SLOT(connectUpdata())); //��������֮��ť��ʹ��
        connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(readMassage())); //��ȡ���յ���Ϣ
    }
    else if (ui.connectRelayButton->text() == "�Ͽ�")
    {
        //��ʱ���ر�
        if (timer->isActive())//�ж϶�ʱ���Ƿ��ڹ���
            timer->stop();
        if(timer) delete timer;

        tcpSocket->abort();//abort��������ʹ�����������ֹ/�쳣�˳�
        if(tcpSocket) delete tcpSocket;
        tcpSocket = NULL;
        disconnectUpdata();
    }
}

// �ڵ�����Ӻ󣬴������ӣ����޷���������/ʧȥ���ӣ������ú���
void RelayDialog::displayError(QAbstractSocket::SocketError)
{
    QMessageBox::warning(this, tr("Warnning"), tcpSocket->errorString());
    tcpSocket->close();

    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(255,0,0);"//��ɫ
        "border: 2px solid rgb(178, 34, 34);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(255, 165, 0);"
        "}");

    ui.NetStatusLabel->setText("�޷�����");
    ui.connectRelayButton->setText("����"); // û�����ӵ��κ����磬���Իָ�������״̬
    ui.connectRelayButton->setEnabled(true); // �������Ӱ�ť�ָ�ʹ��
    ui.controlRelayButton->setEnabled(false); // ���������ؽ�ֹʹ��
    ui.IP_RelayEdit->setEnabled(true); // ������
    ui.Port_RelayEdit->setEnabled(true); // ������
}

// ���ӳɹ���������Ӧ��ť����
void RelayDialog::connectUpdata()
{
    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(0,0,0);" //��ɫ
        "border: 2px solid rgb(54, 100, 139);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(0, 150, 136);"//�ٺ�ɫ
        "}");
    ui.NetStatusLabel->setText("���ӳɹ�");

    // ������ӳɹ���ʵ�ְ�ť��ת
    ui.connectRelayButton->setText("�Ͽ�");
    ui.controlRelayButton->setEnabled(true); // ���������ؿ���ʹ��
    ui.connectRelayButton->setEnabled(true); // �������Ӱ�ť�ָ�ʹ��
    ui.IP_RelayEdit->setEnabled(false); //��ֹ����
    ui.Port_RelayEdit->setEnabled(false); //��ֹ����
    
    // ������ʱ����ѯ�̵���״̬
    timer = new QTimer();
    timer->setInterval(500); //��λ��ms
    timer->start();
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut())); // ���ö�ʱ������ָ���ѯ״̬

}

// �Ͽ����ӣ�������Ӧ��ť����
void RelayDialog::disconnectUpdata()
{
    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(255,0,0);"//��ɫ
        "border: 2px solid rgb(178, 34, 34);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(255, 165, 0);"
        "}");
    ui.NetStatusLabel->setText("δ����");

    // ����Ͽ����ӣ�ʵ�ְ�ť��ת
    ui.connectRelayButton->setText("����");
    ui.controlRelayButton->setEnabled(false); // ���������ؽ�ֹʹ��
    ui.connectRelayButton->setEnabled(true); // �������Ӱ�ť�ָ�ʹ��
    ui.IP_RelayEdit->setEnabled(true);
    ui.Port_RelayEdit->setEnabled(true);
}

// ���Ƽ̵�������
void RelayDialog::on_controlRelayButton_clicked()
{
    if (ui.controlRelayButton->text() == "�ر�") {
        tcpSocket->write(tcp_order.PowerCH1_OFF); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
        tcpSocket->write(tcp_order.PowerCH2_OFF); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
    }
    else if(ui.controlRelayButton->text() == "��") {
        tcpSocket->write(tcp_order.PowerCH1_ON); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
        tcpSocket->write(tcp_order.PowerCH2_ON); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
    }
}

// ��ʱ����ѯ�̵���״̬
void RelayDialog::onTimeOut()
{
    tcpSocket->write(tcp_order.PowerStatus); WaitingSocketWrite();  Sleep(tcp_order.waitingTime);
}

// ��ȡ�̵������͵���Ϣ
void RelayDialog::readMassage()
{
    QByteArray data = tcpSocket->readAll();//��ȡ����
    if (data == "00000000") {
        ui.label_5->setText("�Ѵ�");
        ui.controlRelayButton->setText("�ر�");
    }
    if (data == "11000000") {
        ui.label_5->setText("�ѹر�");
        ui.controlRelayButton->setText("��");
    }
    //ui->showLineEdit->setText(QString(data));//��ʾ����
}


// �ȴ�QTcpSocketд������
// �ȴ�������ϣ����ó�ʱʱ��ms
void RelayDialog::WaitingSocketWrite(int time) {
    if (!tcpSocket->waitForBytesWritten(time)) {
        return;
    }
}