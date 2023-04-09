#ifndef QCPTOOLTIP_H
#define QCPTOOLTIP_H
#include "qcustomplot.h"
#pragma  execution_character_set("utf-8")
class QCPToolTip : public QCPAbstractItem
{
    Q_OBJECT
public:
    explicit QCPToolTip(QCustomPlot* parentPlot);

    void setText(const QString& text);
    void setFont(const QFont& font);
    void setTextColor(const QColor& color);
    void setBorderPen(const QPen& pen);
    void setBrush(const QBrush& brush);
    void setRadius(double xRadius, double yRadius, Qt::SizeMode mode = Qt::AbsoluteSize);
    void setOffset(double xOffset, double yOffset);
    void setPadding(const QMargins& paddings);

    Q_SLOT void handleTriggerEvent(QMouseEvent* event);
    void updatePosition(const QPointF& newPos, bool replot = false);

    void update();

    virtual double selectTest(const QPointF& pos, bool onlySelectable, QVariant* details) const Q_DECL_OVERRIDE;

    QCPItemPosition* const position;
protected:

    bool mPlotReplot;    // 表明是由QCustomPlot刷新的，需要更新位置
    QString mText;
    Qt::Alignment mTextAlignment;
    QFont mFont;
    QColor mTextColor;
    QPen mBorderPen;
    QBrush mBrush;

    QPointF mRadius;
    Qt::SizeMode mSizeMode;

    QPointF mOffset;     // 偏移鼠标的距离
    QMargins mPadding;

    QCPGraph* mHighlightGraph;
    QPointF mGraphDataPos;

    virtual void draw(QCPPainter* painter) Q_DECL_OVERRIDE;
    virtual void drawGraphScatterPlot(QCPPainter* painter, QCPGraph* graph, const QPointF& pos);

    int pickClosest(double target, const QVector<double>& vector);
};
#endif // QCPTOOLTIP_H
