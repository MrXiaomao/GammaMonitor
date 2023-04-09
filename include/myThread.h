#pragma once
#include <QObject>
#include "qcustomplot.h"
class myThread : public QObject
{
    Q_OBJECT
public:
    myThread(QObject* parent = nullptr);

signals:
    void task_signals(QCustomPlot* customplotOnThread);//ʹ�õ������߳�ui�ϵ�customPlot

public slots:
    void draw_slot(QCustomPlot* customPlot);
};

