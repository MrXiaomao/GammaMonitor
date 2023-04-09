#include "mainWindow.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    mainWindow w;
    w.show();
    return a.exec();//直接return 0 就达不到事件循环并显示的效果，程序直接退出了，而exec就是在这里进行了循环一直循环处理着用户和系统的事件。
}
