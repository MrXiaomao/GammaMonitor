#pragma once
#include <QObject>
#include "qcustomplot.h"
class myThread : public QObject
{
    Q_OBJECT
public:
    myThread(QObject* parent = nullptr);

signals:
    void task_signals(QCustomPlot* customplotOnThread);//使用的是主线程ui上的customPlot

public slots:
    void draw_slot(QCustomPlot* customPlot);
};

