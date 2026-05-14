#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QtNetwork>

// 工作线程内的 TCP 收发（与 GUI 线程分离，避免阻塞导致 ARM 数据流中断）
class TcpClientThread : public QObject {
    Q_OBJECT
public:
    explicit TcpClientThread(QObject* parent = nullptr);

public slots:
    void applyHostPort(const QString& host, quint16 port);
    void setOptions(bool heartbeatEnabled, bool autoReconnect);
    void start();
    void stop();
    void sendData(const QByteArray& data);

private slots:
    void connectToHost();
    void onConnected();
    void onError(QAbstractSocket::SocketError error);
    void onDisconnected();
    void processData();

private:
    void scheduleReconnect();
    /// 在工作线程内同步拆除套接字（禁止 deleteLater：线程 quit 后事件循环不再处理，主线程删 worker 会跨线程销毁 QNativeSocketEngine）
    void tearDownSocket();

signals:
    void dataReceived(const QByteArray& data);
    void sigErrorOccurred(QAbstractSocket::SocketError error);
    void connectionStatusChanged(bool connected);

private:
    QTcpSocket* m_socket;
    QTimer* m_reconnectTimer;
    QTimer* m_heartbeatTimer;
    QString m_host;
    quint16 m_port;
    int m_reconnectAttempts = 0;
    bool m_heartbeatEnabled = false;
    bool m_autoReconnect = false;
};

// 主线程使用的 TCP 客户端封装（内部 QThread + TcpClientThread）
class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    void setHeartbeatEnabled(bool on);
    void setAutoReconnect(bool on);

    void connectToHost(const QString& host, quint16 port);
    void disconnectFromHost();
    void send(const QByteArray& data);

    bool isConnected() const { return m_connected; }

signals:
    void startSignal();
    void stopSignal();
    void sigconnectError(QAbstractSocket::SocketError error);
    void sigconnectStatusChanged(bool connected);
    void sendDataSignal(const QByteArray&);
    void dataReceived(const QByteArray&);

private:
    QThread* m_thread;
    TcpClientThread* m_worker;
    bool m_shuttingDown = false;
    bool m_connected = false;
    bool m_heartbeat = false;
    bool m_autoReconnect = false;
};

#endif // TCPCLIENT_H
