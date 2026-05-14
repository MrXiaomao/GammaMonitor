/*
 * @Author: MrPan
 * @Date: 2026-05-14 19:33:27
 * @LastEditors: Maoxiaoqing
 * @LastEditTime: 2026-05-14 21:33:00
 * @Description: 请填写简介
 */
#include "commandhelper.h"

CommandHelper::CommandHelper(QObject *parent)
    : QObject(parent)
    , m_clientArm(new TcpClient(this))
    , m_clientRelay(new TcpClient(this))
{
    m_clientArm->setHeartbeatEnabled(false);
    m_clientArm->setAutoReconnect(false);
    m_clientRelay->setHeartbeatEnabled(false);
    m_clientRelay->setAutoReconnect(false);

    connect(m_clientArm, &TcpClient::dataReceived, this, &CommandHelper::armDataReceived);
    connect(m_clientArm, &TcpClient::sigconnectStatusChanged, this, [this](bool on) {
        if (m_armOnline == on)
            return;
        m_armOnline = on;
        emit armConnectStatusChanged(on);
    });
    connect(m_clientArm, &TcpClient::sigconnectError, this, &CommandHelper::armConnectError);

    connect(m_clientRelay, &TcpClient::dataReceived, this, &CommandHelper::relayDataReceived);
    connect(m_clientRelay, &TcpClient::sigconnectStatusChanged, this, [this](bool on) {
        if (m_relayOnline == on)
            return;
        m_relayOnline = on;
        emit relayConnectStatusChanged(on);
    });
    connect(m_clientRelay, &TcpClient::sigconnectError, this, &CommandHelper::relayConnectError);
}

void CommandHelper::applySettings(const QJsonObject& json)
{
    m_ipArm = json.value(QStringLiteral("IP_Detector")).toString();
    m_portArm = static_cast<quint16>(json.value(QStringLiteral("Port_Detector")).toString().toUInt());
    m_ipRelay = json.value(QStringLiteral("IP_Relay")).toString();
    m_portRelay = static_cast<quint16>(json.value(QStringLiteral("Port_Relay")).toString().toUInt());
}

void CommandHelper::connectArm()
{
    if (!m_clientArm || m_ipArm.isEmpty() || m_portArm == 0)
        return;
    m_clientArm->connectToHost(m_ipArm, m_portArm);
}

void CommandHelper::disconnectArm()
{
    if (!m_clientArm)
        return;
    m_clientArm->disconnectFromHost();
}

void CommandHelper::sendArm(const QByteArray& data)
{
    if (m_clientArm)
        m_clientArm->send(data);
}

void CommandHelper::connectRelay()
{
    if (!m_clientRelay || m_ipRelay.isEmpty() || m_portRelay == 0)
        return;
    m_clientRelay->connectToHost(m_ipRelay, m_portRelay);
}

void CommandHelper::disconnectRelay()
{
    if (!m_clientRelay)
        return;
    m_clientRelay->disconnectFromHost();
}

void CommandHelper::sendRelay(const QByteArray& data)
{
    if (m_clientRelay)
        m_clientRelay->send(data);
}

CommandHelper::~CommandHelper()
{
    if (m_clientArm && m_armOnline)
        m_clientArm->disconnectFromHost();
    if (m_clientRelay && m_relayOnline)
        m_clientRelay->disconnectFromHost();
}
