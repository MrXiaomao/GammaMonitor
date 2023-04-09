
#ifndef MYTRACERLINE_H
#define MYTRACERLINE_H

#include <QObject>
#include "qcustomplot.h"

class myTracerLine : public QObject
{
public:
    enum LineType
    {
        VerticalLine,//垂直线
        HorizonLine, //水平线
        Both//同时显示水平和垂直线
    };
    explicit myTracerLine(QCustomPlot* _plot, LineType _type = VerticalLine, QObject* parent = Q_NULLPTR);
    ~myTracerLine();
    void initLine();
    void updatePosition(double xValue, double yValue);

    void setVisible(bool vis)
    {
        if (m_lineV)
            m_lineV->setVisible(vis);
        if (m_lineH)
            m_lineH->setVisible(vis);
    }

protected:
    bool m_visible;//是否可见
    LineType m_type;//类型
    QCustomPlot* m_plot;//图表
    QCPItemStraightLine* m_lineV; //垂直线
    QCPItemStraightLine* m_lineH; //水平线

};

#endif // MYTRACERLINE_H

