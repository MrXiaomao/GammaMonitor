
#ifndef MYTRACER_H
#define MYTRACER_H

#include <QObject>
#include "qcustomplot.h"
#include "QCPItemTip.h"

enum TracerType
{
    XAxisTracer,
    YAxisTracer,
    DataTracer
};

class myTracer : public QObject
{
    Q_OBJECT
public:
    explicit myTracer(QCustomPlot* _plot, QCPGraph* _graph, TracerType _type);
    ~myTracer();

public:
    void setPen(const QPen& Pen);
    void setBrush(const QBrush& brush);
    void setText(const QString& text);
    void setLabelPen(const QPen& pen);
    void updatePosition(double xValue, double yValue);

    void setVisible(bool visible);
public:
    QCustomPlot* plot;     //ʵ����plot
    QCPGraph* graph;       //��ͼ������
    QCPItemTracer* tracer; //׷�ٵ�
    //QCPItemText* label;   //��ʾ�ı�
    QCPItemTip* label;    //��ʾ�ı�
    QCPItemLine* arrow;    //��ͷ

    TracerType type;
    bool visible;
};

#endif // MYTRACER_H
