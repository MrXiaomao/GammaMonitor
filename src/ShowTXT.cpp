#include "ShowTXT.h"
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>

// �����ı��������ʾ���������»�����
// ���ı���ģ�������������ʾ�����ı����ӿڣ�showTXT(filewholePath)

ShowTXT::ShowTXT(QString title, QString fileName, QWidget *parent)
	: myFileName(fileName), QDialog(parent)
{
	ui.setupUi(this);

	// ��������С����ť
	Qt::WindowFlags flag = windowFlags();
	Qt::WindowFlags flags = flag | Qt::WindowMinMaxButtonsHint;
	setWindowFlags(flags);

    // 1���ж��ļ��Ƿ����
    QFile file(this->myFileName);
    QFileInfo fileInfo(file);
	if (!fileInfo.isFile()){
        QString information = "�ļ�����" + this->myFileName + "��������";
        QMessageBox::warning(NULL, "warning", information, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		return;
	} 

    //2���Զ��ķ�ʽ���ļ�
    bool res = file.open(QIODevice::ReadOnly);
    if (res == false)
    {
        QString information = this->myFileName + "�ļ���ʧ��";
        QMessageBox::warning(NULL, "warning", information, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }

    //3�����ļ�������Ϊ�����༭����
    // readAll�����е���ʽ��ȡ�ļ�
    QString txtInfo = this->myFileName + "\n\n" + QString(file.readAll());
    ui.textBrowser->setText(txtInfo);
    setWindowTitle(title);

    //4���ر��ļ�
    file.close();
}

ShowTXT::~ShowTXT()
{
}
