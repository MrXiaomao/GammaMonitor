#pragma once

#include <QDialog>
#include "ui_RelayDialog.h"
#include <QAbstractSocket>
#include <QPointer>
#include <QTimer>
#include "order.h"

class CommandHelper;

// 继电器控制（TCP 经 CommandHelper / TcpClient 管理）
class RelayDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RelayDialog(CommandHelper* netHelper, QWidget* parent = Q_NULLPTR);
    ~RelayDialog();

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void onRelayBytes(const QByteArray& data);
    void slotNetError(QAbstractSocket::SocketError);
    void onRelayConnectChanged(bool connected);
    void on_connectRelayButton_clicked();
    void on_controlRelayButton_clicked();
    void onTimeOut();

private:
    Ui::RelayDialog ui;
    QPointer<CommandHelper> m_net;

    Order tcp_order;
    QTimer* timer;
};
