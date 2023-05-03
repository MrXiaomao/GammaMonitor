#include "ShowTXT.h"
#include <QFileInfo>
#include <QFile>
#include <QMessageBox>

// 采用文本框进行显示，滑块上下滑动，
// 该文本框模块后续可用于显示其他文本，接口：showTXT(filewholePath)

ShowTXT::ShowTXT(QString title, QString fileName, QWidget *parent)
	: myFileName(fileName), QDialog(parent)
{
	ui.setupUi(this);

	// 添加最大化最小化按钮
	Qt::WindowFlags flag = windowFlags();
	Qt::WindowFlags flags = flag | Qt::WindowMinMaxButtonsHint;
	setWindowFlags(flags);

    // 1、判断文件是否存在
    QFile file(this->myFileName);
    QFileInfo fileInfo(file);
	if (!fileInfo.isFile()){
        QString information = "文件：“" + this->myFileName + "”不存在";
        QMessageBox::warning(NULL, "warning", information, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
		return;
	} 

    //2、以读的方式打开文件
    bool res = file.open(QIODevice::ReadOnly);
    if (res == false)
    {
        QString information = this->myFileName + "文件打开失败";
        QMessageBox::warning(NULL, "warning", information, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return;
    }

    //3、读文件：设置为读到编辑区域
    // readAll是以行的形式读取文件
    QString txtInfo = this->myFileName + "\n\n" + QString(file.readAll());
    ui.textBrowser->setText(txtInfo);
    setWindowTitle(title);

    //4、关闭文件
    file.close();
}

ShowTXT::~ShowTXT()
{
}
