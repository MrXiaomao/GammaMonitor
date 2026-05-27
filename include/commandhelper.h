#ifndef COMMANDHELPER_H
#define COMMANDHELPER_H

#include <QObject>
#include <QJsonObject>
#include <QPointer>
#include <QAbstractSocket>
#include <QByteArray>
#include <QMap>
#include <QThread>
#include <QTimer>
#include "tcpclient.h"

using ArmCoefMap = QMap<int, double>;
Q_DECLARE_METATYPE(ArmCoefMap)

/// Parsed one ARM 40-byte frame (for main window UI update).
struct ArmFrameData
{
    int equipmentId = 0;
    int counter[4] = {0, 0, 0, 0};
    double temperature = 0.0;
    double inputVolt = 0.0;
    double voltA = 0.0;
    double voltB = 0.0;
};
Q_DECLARE_METATYPE(ArmFrameData)

struct CommandItem
{
    QString name;           // 指令名称（中文或英文描述）
    QByteArray cmdPayload;  // 实际发送的指令内容（避免与 Windows/第三方宏 data 冲突）

    CommandItem() {}
    CommandItem(const QString &n, const QByteArray &d)
        : name(n), cmdPayload(d) {}
};

/// Framing + field decode on a dedicated thread (no QWidget).
class ArmDataParser : public QObject
{
    Q_OBJECT
public:
    explicit ArmDataParser(QObject* parent = nullptr);

public slots:
    void applyCoefficients(ArmCoefMap siPmA, ArmCoefMap siPmB, ArmCoefMap inputVolt);
    void reset();
    void parseChunk(const QByteArray& chunk);

signals:
    void frameParsed(const ArmFrameData& frame);

private:
    static void parseCounters(const QByteArray& frame, int* count);
    static double parseTemperature(const QByteArray& frame);
    static double parseInputVolt(const QByteArray& frame, int equipmentId, const ArmCoefMap& coef);
    static double parseVoltA(const QByteArray& frame, int equipmentId, const ArmCoefMap& coef);
    static double parseVoltB(const QByteArray& frame, int equipmentId, const ArmCoefMap& coef);

    QByteArray m_buffer;
    ArmCoefMap m_coefSiPMA;
    ArmCoefMap m_coefSiPMB;
    ArmCoefMap m_coefInputVolt;
};

/// ARM + relay TCP; ARM raw bytes from TcpClient, parse on ArmDataParser thread.
class CommandHelper : public QObject
{
    Q_OBJECT
public:
    explicit CommandHelper(QObject* parent = nullptr);
    ~CommandHelper();

    void applySettings(const QJsonObject& json);

    void connectArm();
    void disconnectArm();

    void connectRelay();
    void disconnectRelay();
    void sendRelay(const QByteArray& data);

    bool isArmConnected() const { return m_armOnline; }
    bool isRelayConnected() const { return m_relayOnline; }

    /// Push voltage coeffs from main window (call from GUI thread).
    void setArmParserCoefficients(const ArmCoefMap& siPmA,
                                  const ArmCoefMap& siPmB,
                                  const ArmCoefMap& inputVolt);
    /// Clear sticky buffer (start measure, disconnect ARM, etc.).
    void resetArmParserBuffer();
    void enqueueArmCommand(CommandItem cmd);
    void sendNextArmCommand();

private:
    void sendArm(const QByteArray& data);

signals:
    void sigAppendMsg(const QString& msg, QtMsgType msgType);

    void armDataParsed(const ArmFrameData& frame);
    void armConnectStatusChanged(bool connected);
    void armConnectError(QAbstractSocket::SocketError error);

    void relayDataReceived(const QByteArray& data);
    void relayConnectStatusChanged(bool connected);
    void relayConnectError(QAbstractSocket::SocketError error);

private:
    QPointer<TcpClient> m_clientArm;
    QPointer<TcpClient> m_clientRelay;

    QThread* m_armParserThread = nullptr;
    ArmDataParser* m_armParser = nullptr;

    QString m_ipArm;
    quint16 m_portArm = 0;
    QString m_ipRelay;
    quint16 m_portRelay = 0;

    bool m_armOnline = false;
    bool m_relayOnline = false;
    
    QVector<CommandItem> m_cmdArmItems;
    QTimer *m_cmdArmTimer = nullptr;
    bool m_Armsending = false;
    bool m_disconnectingArm = false;
};

#endif // COMMANDHELPER_H
