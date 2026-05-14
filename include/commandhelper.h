#ifndef COMMANDHELPER_H
#define COMMANDHELPER_H

#include <QObject>
#include <QJsonObject>
#include <QPointer>
#include <QAbstractSocket>
#include "tcpclient.h"

/// 统一管理 ARM 探测器与继电器两路 TCP（底层为 TcpClient 工作线程收发）
class CommandHelper : public QObject
{
    Q_OBJECT
public:
    explicit CommandHelper(QObject *parent = nullptr);
    ~CommandHelper();

    /// 从 GammaMonitor 的 setting.json（ReadSetting）字段更新地址
    void applySettings(const QJsonObject& json);

    void connectArm();
    void disconnectArm();
    void sendArm(const QByteArray& data);

    void connectRelay();
    void disconnectRelay();
    void sendRelay(const QByteArray& data);

    bool isArmConnected() const { return m_armOnline; }
    bool isRelayConnected() const { return m_relayOnline; }

signals:
    void sigAppendMsg(const QString& msg, QtMsgType msgType);

    void armDataReceived(const QByteArray& data);
    void armConnectStatusChanged(bool connected);
    void armConnectError(QAbstractSocket::SocketError error);

    void relayDataReceived(const QByteArray& data);
    void relayConnectStatusChanged(bool connected);
    void relayConnectError(QAbstractSocket::SocketError error);

private:
    QPointer<TcpClient> m_clientArm;
    QPointer<TcpClient> m_clientRelay;

    QString m_ipArm;
    quint16 m_portArm = 0;
    QString m_ipRelay;
    quint16 m_portRelay = 0;

    bool m_armOnline = false;
    bool m_relayOnline = false;
};

#endif // COMMANDHELPER_H
