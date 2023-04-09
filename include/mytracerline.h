
#ifndef MYTRACERLINE_H
#define MYTRACERLINE_H

#include <QObject>
#include "qcustomplot.h"

class myTracerLine : public QObject
{
public:
    enum LineType
    {
        VerticalLine,//��ֱ��
        HorizonLine, //ˮƽ��
        Both//ͬʱ��ʾˮƽ�ʹ�ֱ��
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
    bool m_visible;//�Ƿ�ɼ�
    LineType m_type;//����
    QCustomPlot* m_plot;//ͼ��
    QCPItemStraightLine* m_lineV; //��ֱ��
    QCPItemStraightLine* m_lineH; //ˮƽ��

};

#endif // MYTRACERLINE_H

