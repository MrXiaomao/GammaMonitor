#pragma once

#include <QDialog>
#include "ui_ShowTXT.h"

class ShowTXT : public QDialog
{
	Q_OBJECT

public:
	ShowTXT(QString title, QString fileName, QWidget *parent = Q_NULLPTR);
	~ShowTXT();

private:
	Ui::ShowTXT ui;
	QString myFileName;
};
