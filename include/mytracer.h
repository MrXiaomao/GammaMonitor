
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
    QCustomPlot* plot;     //实例化plot
    QCPGraph* graph;       //绘图层曲线
    QCPItemTracer* tracer; //追踪点
    //QCPItemText* label;   //显示文本
    QCPItemTip* label;    //显示文本
    QCPItemLine* arrow;    //箭头

    TracerType type;
    bool visible;
};

#endif // MYTRACER_H
