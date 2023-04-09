#pragma once

#include <QDialog>
#include "ui_SetThreshlod.h"
#include <QTcpSocket>

class SetThreshlod : public QDialog
{
	Q_OBJECT

public:
	SetThreshlod(QTcpSocket* tcpSocket, QWidget *parent = Q_NULLPTR);
	~SetThreshlod();

private slots:
	void on_SetButton_clicked(); // –ﬁ∏ƒ≈‰÷√
	
private:
	Ui::SetThreshlod ui;
	QTcpSocket* parentSocket;
};
