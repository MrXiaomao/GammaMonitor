#include "RelayDialog.h"
#include "commandhelper.h"
#include "mainWindow.h"

#include <QIntValidator>
#include <QMessageBox>

#pragma execution_character_set("utf-8")

RelayDialog::RelayDialog(CommandHelper* netHelper, QWidget* parent)
    : QDialog(parent)
    , m_net(netHelper)
    , timer(Q_NULLPTR)
{
    ui.setupUi(this);
    Q_ASSERT(m_net != nullptr);

    QJsonObject jsonSetting = mainWindow::ReadSetting();
    const QString tcpIp = jsonSetting["IP_Relay"].toString();
    const QString tcpPort = jsonSetting["Port_Relay"].toString();

    ui.controlRelayButton->setEnabled(false);
    ui.IP_RelayEdit->setIP(tcpIp);
    ui.Port_RelayEdit->setText(tcpPort);
    ui.Port_RelayEdit->setValidator(new QIntValidator(1, 9999, this));
}

RelayDialog::~RelayDialog()
{
    if (timer) {
        if (timer->isActive())
            timer->stop();
        delete timer;
        timer = Q_NULLPTR;
    }
    // if (m_net && m_net->isRelayConnected()) {
    //     disconnect(m_net, nullptr, this, nullptr);
    //     m_net->disconnectRelay();
    //     qInfo().noquote() << "[~RelayDialog]继电器连接已断开";
    // }
}

void RelayDialog::closeEvent(QCloseEvent* event)
{
    if (timer) {
        if (timer->isActive())
            timer->stop();
        delete timer;
        timer = Q_NULLPTR;
    }
    if (m_net && m_net->isRelayConnected()) {
        disconnect(m_net, nullptr, this, nullptr);
        m_net->disconnectRelay();
        qInfo().noquote() << "继电器连接已断开";
    }
    qInfo().noquote() << "继电器控制界面已关闭";
    event->accept();
}

void RelayDialog::on_connectRelayButton_clicked()
{
    if (ui.connectRelayButton->text() == "连接")
    {
        ui.connectRelayButton->setEnabled(false);
        ui.NetStatusLabel->setText("网络连接中。。。");

        const QString tcpIp = ui.IP_RelayEdit->getIP();
        const QString tcpPort = ui.Port_RelayEdit->text();

        QJsonObject jsonSetting = mainWindow::ReadSetting();
        jsonSetting["IP_Relay"] = tcpIp;
        jsonSetting["Port_Relay"] = tcpPort;
        mainWindow::WriteSetting(jsonSetting);

        disconnect(m_net, nullptr, this, nullptr);
        connect(m_net, &CommandHelper::relayConnectStatusChanged, this, &RelayDialog::onRelayConnectChanged, Qt::UniqueConnection);
        connect(m_net, &CommandHelper::relayConnectError, this, &RelayDialog::slotNetError, Qt::UniqueConnection);
        connect(m_net, &CommandHelper::relayDataReceived, this, &RelayDialog::onRelayBytes, Qt::UniqueConnection);

        m_net->applySettings(mainWindow::ReadSetting());
        m_net->connectRelay();
    }
    else if (ui.connectRelayButton->text() == "断开")
    {
        if (timer) {
            if (timer->isActive())
                timer->stop();
            delete timer;
            timer = Q_NULLPTR;
        }
        if (m_net && m_net->isRelayConnected()) {
            m_net->disconnectRelay();
        } else {
            onRelayConnectChanged(false);
        }
    }
}

void RelayDialog::slotNetError(QAbstractSocket::SocketError)
{
    QMessageBox::warning(this, tr("Warnning"), tr("继电器 TCP 连接错误"));

    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(255,0,0);"
        "border: 2px solid rgb(178, 34, 34);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(255, 165, 0);"
        "}");

    ui.NetStatusLabel->setText("无法连接");
    ui.connectRelayButton->setText("连接");
    ui.connectRelayButton->setEnabled(true);
    ui.controlRelayButton->setEnabled(false);
    ui.IP_RelayEdit->setEnabled(true);
    ui.Port_RelayEdit->setEnabled(true);
}

void RelayDialog::onRelayConnectChanged(bool connected)
{
    if (!connected) {
        if (timer) {
            if (timer->isActive())
                timer->stop();
            delete timer;
            timer = Q_NULLPTR;
        }
        ui.NetStatusLabel->setStyleSheet(
            "QLineEdit{"
            "color:rgba(255,0,0);"
            "border: 2px solid rgb(178, 34, 34);"
            "}"
            "QLineEdit:hover{"
            "border: 2px solid rgb(255, 165, 0);"
            "}");
        qInfo().noquote() << "继电器断开连接";
        ui.NetStatusLabel->setText("未连接");
        ui.connectRelayButton->setText("连接");
        ui.controlRelayButton->setEnabled(false);
        ui.connectRelayButton->setEnabled(true);
        ui.IP_RelayEdit->setEnabled(true);
        ui.Port_RelayEdit->setEnabled(true);
        return;
    }

    qInfo().noquote() << "继电器连接成功";
    ui.NetStatusLabel->setStyleSheet(
        "QLineEdit{"
        "color:rgba(0,0,0);"
        "border: 2px solid rgb(54, 100, 139);"
        "}"
        "QLineEdit:hover{"
        "border: 2px solid rgb(0, 150, 136);"
        "}");
    ui.NetStatusLabel->setText("连接成功");

    ui.connectRelayButton->setText("断开");
    ui.controlRelayButton->setEnabled(true);
    ui.connectRelayButton->setEnabled(true);
    ui.IP_RelayEdit->setEnabled(false);
    ui.Port_RelayEdit->setEnabled(false);

    timer = new QTimer(this);
    timer->setInterval(500);
    connect(timer, &QTimer::timeout, this, &RelayDialog::onTimeOut);
    timer->start();
}

void RelayDialog::on_controlRelayButton_clicked()
{
    if (!m_net || !m_net->isRelayConnected()) {
        onRelayConnectChanged(false);
        return;
    }

    const bool turnOff = (ui.controlRelayButton->text() == "关闭");
    const bool turnOn = (ui.controlRelayButton->text() == "打开");
    if (!turnOff && !turnOn)
        return;

    const QByteArray firstCmd = turnOff ? QByteArray(tcp_order.PowerCH1_OFF) : QByteArray(tcp_order.PowerCH1_ON);
    const QByteArray secondCmd = turnOff ? QByteArray(tcp_order.PowerCH2_OFF) : QByteArray(tcp_order.PowerCH2_ON);
    const int relayCommandIntervalMs = 50;

    qInfo().noquote() << (turnOff ? "发送关闭探测器电源指令" : "发送打开探测器电源指令");

    if (timer && timer->isActive())
        timer->stop();
    ui.controlRelayButton->setEnabled(false);

    m_net->sendRelay(firstCmd);
    QTimer::singleShot(relayCommandIntervalMs, this, [this, secondCmd]() {
        if (m_net && m_net->isRelayConnected()) {
            m_net->sendRelay(secondCmd);
            ui.controlRelayButton->setEnabled(true);
        }
    });

    if (timer)
        timer->start();
    /*QTimer::singleShot(relayCommandIntervalMs * 2, this, [this]() {
        ui.controlRelayButton->setEnabled(true);
        if (m_net && m_net->isRelayConnected()) {
            m_net->sendRelay(tcp_order.PowerStatus);
            if (timer)
                timer->start();
        } else {
            onRelayConnectChanged(false);
        }
    });*/
}
void RelayDialog::onTimeOut()
{
    if (!m_net)
        return;
    m_net->sendRelay(tcp_order.PowerStatus);
}

void RelayDialog::onRelayBytes(const QByteArray& data)
{
    if (data == "00000000") {
        ui.label_5->setText("已打开");
        ui.controlRelayButton->setText("关闭");
    }
    if (data == "11000000") {
        ui.label_5->setText("已关闭");
        ui.controlRelayButton->setText("打开");
    }
}


