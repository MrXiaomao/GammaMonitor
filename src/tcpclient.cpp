#include "tcpclient.h"
#include <QThread>
#include <QtGlobal>

TcpClientThread::TcpClientThread(QObject* parent)
    : QObject(parent)
    , m_socket(nullptr)
    , m_reconnectTimer(new QTimer(this))
    , m_heartbeatTimer(new QTimer(this))
{
    m_reconnectTimer->setSingleShot(true);
    connect(m_reconnectTimer, &QTimer::timeout, this, &TcpClientThread::connectToHost);

    connect(m_heartbeatTimer, &QTimer::timeout, this, [this]() {
        if (!m_heartbeatEnabled)
            return;
        if (m_socket && m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->write("HEARTBEAT");
        }
    });
}

void TcpClientThread::applyHostPort(const QString& host, quint16 port)
{
    m_host = host;
    m_port = port;
}

void TcpClientThread::setOptions(bool heartbeatEnabled, bool autoReconnect)
{
    m_heartbeatEnabled = heartbeatEnabled;
    m_autoReconnect = autoReconnect;
}

void TcpClientThread::start()
{
    connectToHost();
    if (m_heartbeatEnabled)
        m_heartbeatTimer->start(5000);
}

void TcpClientThread::tearDownSocket()
{
    if (!m_socket)
        return;
    QObject::disconnect(m_socket, nullptr, this, nullptr);
    m_socket->abort();
    delete m_socket;
    m_socket = nullptr;
}

void TcpClientThread::stop()
{
    m_reconnectTimer->stop();
    m_heartbeatTimer->stop();
    tearDownSocket();
    emit connectionStatusChanged(false);
}

void TcpClientThread::sendData(const QByteArray& data)
{
    if (!m_socket || m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "TcpClientThread: send while disconnected";
        return;
    }
    const int chunkSize = 4096;
    for (int i = 0; i < data.size(); i += chunkSize) {
        m_socket->write(data.mid(i, chunkSize));
    }
}

void TcpClientThread::connectToHost()
{
    tearDownSocket();

    m_socket = new QTcpSocket(this);
    m_socket->setProxy(QNetworkProxy::NoProxy);

    const int bufferSize = 4 * 1024 * 1024;
    m_socket->setSocketOption(QAbstractSocket::SendBufferSizeSocketOption, bufferSize);
    m_socket->setSocketOption(QAbstractSocket::ReceiveBufferSizeSocketOption, bufferSize);

    connect(m_socket, &QTcpSocket::connected, this, &TcpClientThread::onConnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &TcpClientThread::processData);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
    connect(m_socket, &QTcpSocket::errorOccurred, this, &TcpClientThread::onError);
#else
    connect(m_socket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error),
            this, &TcpClientThread::onError);
#endif
    connect(m_socket, &QTcpSocket::disconnected, this, &TcpClientThread::onDisconnected);

    m_socket->connectToHost(m_host, m_port);
}

void TcpClientThread::onConnected()
{
    m_reconnectAttempts = 0;
    emit connectionStatusChanged(true);
}

void TcpClientThread::onError(QAbstractSocket::SocketError error)
{
    if (m_socket)
        qWarning() << "TcpClientThread socket error:" << error << m_socket->errorString();
    emit sigErrorOccurred(error);
    if (m_autoReconnect)
        scheduleReconnect();
}

void TcpClientThread::onDisconnected()
{
    emit connectionStatusChanged(false);
}

void TcpClientThread::processData()
{
    if (!m_socket)
        return;
    const QByteArray chunk = m_socket->readAll();
    if (!chunk.isEmpty())
        emit dataReceived(chunk);
}

void TcpClientThread::scheduleReconnect()
{
    const int maxAttempts = 2;
    if (m_reconnectAttempts >= maxAttempts) {
        qWarning() << "TcpClientThread: reconnect stopped after" << maxAttempts << "attempts";
        return;
    }
    const int maxDelay = 30000;
    const int delay = qMin(1000 * (1 << m_reconnectAttempts), maxDelay);
    m_reconnectTimer->start(delay);
    ++m_reconnectAttempts;
    qInfo() << "TcpClientThread: reconnect in" << delay << "ms";
}

TcpClient::TcpClient(QObject *parent)
    : QObject{parent}
{
    qRegisterMetaType<QAbstractSocket::SocketError>();

    m_thread = new QThread;
    m_worker = new TcpClientThread;
    m_worker->moveToThread(m_thread);

    connect(this, &TcpClient::startSignal, m_worker, &TcpClientThread::start, Qt::QueuedConnection);
    connect(this, &TcpClient::stopSignal, m_worker, &TcpClientThread::stop, Qt::QueuedConnection);
    connect(this, &TcpClient::sendDataSignal, m_worker, &TcpClientThread::sendData, Qt::QueuedConnection);
    connect(m_worker, &TcpClientThread::dataReceived, this, &TcpClient::dataReceived);
    connect(m_worker, &TcpClientThread::connectionStatusChanged, this, [this](bool c) {
        m_connected = c;
        emit sigconnectStatusChanged(c);
    });
    connect(m_worker, &TcpClientThread::sigErrorOccurred, this, &TcpClient::sigconnectError);

    m_thread->start();
}

TcpClient::~TcpClient()
{
    m_shuttingDown = true;
    // 先断开 worker -> TcpClient，避免线程里 emit 的 QueuedConnection 槽在对象析构后投递到主线程
    if (m_worker) {
        QObject::disconnect(m_worker, nullptr, this, nullptr);
        QObject::disconnect(this, nullptr, m_worker, nullptr);
        QMetaObject::invokeMethod(m_worker, "stop", Qt::BlockingQueuedConnection);
    }
    if (m_thread) {
        m_thread->quit();
        m_thread->wait(10000);
    }
    delete m_worker;
    m_worker = nullptr;
    delete m_thread;
    m_thread = nullptr;
}

void TcpClient::setHeartbeatEnabled(bool on)
{
    m_heartbeat = on;
    QMetaObject::invokeMethod(m_worker, "setOptions", Qt::QueuedConnection,
                              Q_ARG(bool, m_heartbeat), Q_ARG(bool, m_autoReconnect));
}

void TcpClient::setAutoReconnect(bool on)
{
    m_autoReconnect = on;
    QMetaObject::invokeMethod(m_worker, "setOptions", Qt::QueuedConnection,
                              Q_ARG(bool, m_heartbeat), Q_ARG(bool, m_autoReconnect));
}

void TcpClient::connectToHost(const QString& host, quint16 port)
{
    QMetaObject::invokeMethod(m_worker, "applyHostPort", Qt::BlockingQueuedConnection,
                              Q_ARG(QString, host), Q_ARG(quint16, port));
    emit startSignal();
}

void TcpClient::disconnectFromHost()
{
    if (m_shuttingDown)
        return;
    // 不在此读取 m_connected：退出/析构阶段可能与 worker 上的状态短暂不一致。
    // stop() 在 worker 上对未连接状态是安全的，应始终投递。
    if (!m_worker || !m_thread)
        return;
    emit stopSignal();
}

void TcpClient::send(const QByteArray& data)
{
    emit sendDataSignal(data);
}
