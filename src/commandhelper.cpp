#include "commandhelper.h"

#include <QtGlobal>
#include <QMetaType>

namespace {

constexpr int kStandardPackLength = 40;
constexpr int kMaxStickyBuffer = 4096;

} // namespace

// --- ArmDataParser ---

ArmDataParser::ArmDataParser(QObject* parent)
    : QObject(parent)
{
}

void ArmDataParser::applyCoefficients(ArmCoefMap siPmA, ArmCoefMap siPmB, ArmCoefMap inputVolt)
{
    m_coefSiPMA = std::move(siPmA);
    m_coefSiPMB = std::move(siPmB);
    m_coefInputVolt = std::move(inputVolt);
}

void ArmDataParser::reset()
{
    m_buffer.clear();
}

void ArmDataParser::parseChunk(const QByteArray& chunk)
{
    m_buffer += chunk;

    for (;;) {
        if (m_buffer.size() < 2)
            return;

        int headIndex = -1;
        for (int i = 0; i < m_buffer.size() - 1; ++i) {
            if ((m_buffer.at(i) & 0xFF) == 0xFF && (m_buffer.at(i + 1) & 0xFF) == 0xFE) {
                headIndex = i;
                break;
            }
        }
        if (headIndex == -1) {
            if (m_buffer.size() > kMaxStickyBuffer)
                m_buffer.remove(0, m_buffer.size() - 255);
            return;
        }
        if (headIndex > 0)
            m_buffer.remove(0, headIndex);

        int tailIndex = -1;
        for (int i = 2; i < m_buffer.size() - 1; ++i) {
            if ((m_buffer.at(i) & 0xFF) == 0xFF && (m_buffer.at(i + 1) & 0xFF) == 0xFD) {
                tailIndex = i;
                break;
            }
        }
        if (tailIndex == -1)
            return;

        const int frameBytes = tailIndex + 2;
        if (frameBytes != kStandardPackLength) {
            m_buffer.remove(0, 2);
            continue;
        }

        const QByteArray onePack = m_buffer.left(kStandardPackLength);
        m_buffer.remove(0, kStandardPackLength);

        ArmFrameData out;
        out.equipmentId = static_cast<quint8>(onePack.at(18));
        out.temperature = parseTemperature(onePack);
        out.inputVolt = parseInputVolt(onePack, out.equipmentId, m_coefInputVolt);
        out.voltA = parseVoltA(onePack, out.equipmentId, m_coefSiPMA);
        out.voltB = parseVoltB(onePack, out.equipmentId, m_coefSiPMB);
        parseCounters(onePack, out.counter);
        // qDebug().nospace() << ", temp=" << out.temperature
        //            << ", inputVolt=" << out.inputVolt
        //            << ", voltA=" << out.voltA
        //            << ", voltB=" << out.voltB
        //            << ", counters=[" << out.counter[0] << "," << out.counter[1] << "," << out.counter[2] << "," << out.counter[3] << "]";
        emit frameParsed(out);
    }
}

void ArmDataParser::parseCounters(const QByteArray& dataPack, int* count)
{
    if (dataPack.size() < 16) {
        count[0] = count[1] = count[2] = count[3] = 0;
        return;
    }
    int i = 2;
    (void)(dataPack.at(i++) & 0xFF); // detector id 1
    int high = dataPack.at(i++) & 0xFF;
    int middle = dataPack.at(i++) & 0xFF;
    int low = dataPack.at(i++) & 0xFF;
    count[0] = (high << 16) | (middle << 8) | low;

    (void)(dataPack.at(i++) & 0xFF);
    high = dataPack.at(i++) & 0xFF;
    middle = dataPack.at(i++) & 0xFF;
    low = dataPack.at(i++) & 0xFF;
    count[1] = (high << 16) | (middle << 8) | low;

    (void)(dataPack.at(i++) & 0xFF);
    high = dataPack.at(i++) & 0xFF;
    middle = dataPack.at(i++) & 0xFF;
    low = dataPack.at(i++) & 0xFF;
    count[2] = (high << 16) | (middle << 8) | low;

    (void)(dataPack.at(i++) & 0xFF);
    high = dataPack.at(i++) & 0xFF;
    middle = dataPack.at(i++) & 0xFF;
    low = dataPack.at(i++) & 0xFF;
    count[3] = (high << 16) | (middle << 8) | low;
}

double ArmDataParser::parseTemperature(const QByteArray& dataPack)
{
    if (dataPack.size() < 21)
        return 0.0;
    const quint8 high = static_cast<quint8>(dataPack.at(19));
    const quint8 low = static_cast<quint8>(dataPack.at(20));
    unsigned short temp = (high << 8) | low;

    double temperature = 0.0;
    if (temp & 0x8000) {
        const unsigned short original = static_cast<unsigned short>((~temp) + 1);
        const int int_part = (original >> 7) & 0xFF;
        const int frac_part = original & 0x7F;
        temperature = -(int_part + frac_part / 128.0);
    } else {
        const int int_part = (temp >> 7) & 0xFF;
        const int frac_part = temp & 0x7F;
        temperature = int_part + frac_part / 128.0;
    }
    return temperature;
}

double ArmDataParser::parseInputVolt(const QByteArray& dataPack, int equipmentId, const ArmCoefMap& coef)
{
    if (dataPack.size() < 23)
        return 0.0;
    const quint8 high = static_cast<quint8>(dataPack.at(21));
    const quint8 low = static_cast<quint8>(dataPack.at(22));
    const int outervoltage_adc = (high << 8) | low;

    double coefficient = 0.002762;
    const auto it = coef.find(equipmentId);
    if (it != coef.end())
        coefficient = it.value();

    return outervoltage_adc * coefficient;
}

double ArmDataParser::parseVoltA(const QByteArray& dataPack, int equipmentId, const ArmCoefMap& coef)
{
    if (dataPack.size() < 25)
        return 0.0;
    const quint8 high = static_cast<quint8>(dataPack.at(23));
    const quint8 low = static_cast<quint8>(dataPack.at(24));
    const int adc = (high << 8) | low;

    double coefficient = 0.023297;
    const auto it = coef.find(equipmentId);
    if (it != coef.end())
        coefficient = it.value();

    return adc * coefficient;
}

double ArmDataParser::parseVoltB(const QByteArray& dataPack, int equipmentId, const ArmCoefMap& coef)
{
    if (dataPack.size() < 27)
        return 0.0;
    const quint8 high = static_cast<quint8>(dataPack.at(25));
    const quint8 low = static_cast<quint8>(dataPack.at(26));
    const int adc = (high << 8) | low;

    double coefficient = 0.023297;
    const auto it = coef.find(equipmentId);
    if (it != coef.end())
        coefficient = it.value();

    return adc * coefficient;
}

// --- CommandHelper ---

CommandHelper::CommandHelper(QObject* parent)
    : QObject(parent)
    , m_clientArm(new TcpClient(this))
    , m_clientRelay(new TcpClient(this))
    , m_armParserThread(new QThread())
    , m_armParser(new ArmDataParser())
{
    qRegisterMetaType<ArmFrameData>();
    qRegisterMetaType<ArmCoefMap>();

    m_clientArm->setHeartbeatEnabled(false);
    m_clientArm->setAutoReconnect(false);
    m_clientRelay->setHeartbeatEnabled(false);
    m_clientRelay->setAutoReconnect(false);

    m_armParser->moveToThread(m_armParserThread);

    connect(m_clientArm, &TcpClient::dataReceived, m_armParser, &ArmDataParser::parseChunk, Qt::QueuedConnection);
    connect(m_armParser, &ArmDataParser::frameParsed, this, [this](const ArmFrameData& d) {
        emit armDataParsed(d);
    }, Qt::QueuedConnection);

    connect(m_clientArm, &TcpClient::sigconnectStatusChanged, this, [this](bool on) {
        if (m_armOnline == on)
            return;
        m_armOnline = on;
        emit armConnectStatusChanged(on);
    });
    connect(m_clientArm, &TcpClient::sigconnectError, this, &CommandHelper::armConnectError);

    connect(m_clientRelay, &TcpClient::dataReceived, this, [this](const QByteArray& d) {
        emit relayDataReceived(d);
    });
    connect(m_clientRelay, &TcpClient::sigconnectStatusChanged, this, [this](bool on) {
        if (m_relayOnline == on)
            return;
        m_relayOnline = on;
        emit relayConnectStatusChanged(on);
    });
    connect(m_clientRelay, &TcpClient::sigconnectError, this, &CommandHelper::relayConnectError);

    m_armParserThread->start();
}

CommandHelper::~CommandHelper()
{
    if (m_clientArm && m_armParser)
        disconnect(m_clientArm, nullptr, m_armParser, nullptr);

    if (m_armParserThread) {
        if (m_armParser)
            QMetaObject::invokeMethod(m_armParser, "reset", Qt::BlockingQueuedConnection);
        m_armParserThread->quit();
        m_armParserThread->wait(5000);
        delete m_armParser;
        m_armParser = nullptr;
        delete m_armParserThread;
        m_armParserThread = nullptr;
    }

    if (m_clientArm && m_armOnline)
        m_clientArm->disconnectFromHost();
    if (m_clientRelay && m_relayOnline)
        m_clientRelay->disconnectFromHost();
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
    if (m_armParser)
        QMetaObject::invokeMethod(m_armParser, "reset", Qt::BlockingQueuedConnection);
    m_clientArm->connectToHost(m_ipArm, m_portArm);
}

void CommandHelper::disconnectArm()
{
    if (m_armParser)
        QMetaObject::invokeMethod(m_armParser, "reset", Qt::BlockingQueuedConnection);
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

void CommandHelper::setArmParserCoefficients(const ArmCoefMap& siPmA,
                                             const ArmCoefMap& siPmB,
                                             const ArmCoefMap& inputVolt)
{
    if (!m_armParser)
        return;
    QMetaObject::invokeMethod(m_armParser, "applyCoefficients", Qt::BlockingQueuedConnection,
                              Q_ARG(ArmCoefMap, siPmA),
                              Q_ARG(ArmCoefMap, siPmB),
                              Q_ARG(ArmCoefMap, inputVolt));
}

void CommandHelper::resetArmParserBuffer()
{
    if (!m_armParser)
        return;
    QMetaObject::invokeMethod(m_armParser, "reset", Qt::QueuedConnection);
}
