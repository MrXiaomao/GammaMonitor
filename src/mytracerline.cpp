#include "mytracerline.h"

myTracerLine::myTracerLine(QCustomPlot* _plot, LineType _type, QObject* parent)
    : QObject(parent),
    m_type(_type),
    m_plot(_plot)
{
    m_lineV = Q_NULLPTR;
    m_lineH = Q_NULLPTR;
    initLine();
}

myTracerLine::~myTracerLine()
{
    if (m_plot)
    {
        if (m_lineV)
            m_plot->removeItem(m_lineV);
        if (m_lineH)
            m_plot->removeItem(m_lineH);
    }
}

void myTracerLine::initLine()
{
    if (m_plot)
    {
        QPen linesPen(QColor("#FFFF00"), 0.5, Qt::SolidLine);

        if (VerticalLine == m_type || Both == m_type)
        {
            m_lineV = new QCPItemStraightLine(m_plot);//垂直线
            m_lineV->setLayer("overlay");
            m_lineV->setPen(linesPen);
            m_lineV->setClipToAxisRect(true);
            m_lineV->point1->setCoords(0, 0);
            m_lineV->point2->setCoords(0, 0);
        }

        if (HorizonLine == m_type || Both == m_type)
        {

            m_lineH = new QCPItemStraightLine(m_plot);//水平线
            m_lineH->setLayer("overlay");
            m_lineH->setPen(linesPen);
            m_lineH->setClipToAxisRect(true);
            m_lineH->point1->setCoords(0, 0);
            m_lineH->point2->setCoords(0, 0);
        }
    }
}

void myTracerLine::updatePosition(double xValue, double yValue)
{
    if (VerticalLine == m_type || Both == m_type)
    {
        if (m_lineV)
        {
            m_lineV->point1->setCoords(xValue, m_plot->yAxis->range().lower);
            m_lineV->point2->setCoords(xValue, m_plot->yAxis->range().upper);
        }
    }

    if (HorizonLine == m_type || Both == m_type)
    {
        if (m_lineH)
        {
            m_lineH->point1->setCoords(m_plot->xAxis->range().lower, yValue);
            m_lineH->point2->setCoords(m_plot->xAxis->range().upper, yValue);

        }
    }
}

