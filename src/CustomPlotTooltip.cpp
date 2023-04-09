#include "CustomPlotTooltip.h"

//我们将ToolTip移入overlay层，原因是因为它可能需要频繁的刷新，这样我们可以不重新绘制图表，优化速度。
//同时将position的位置设置为像素的方式
QCPToolTip::QCPToolTip(QCustomPlot* parentPlot)
    : QCPAbstractItem(parentPlot),
    position(createPosition(QLatin1String("position"))),
    mPlotReplot(true),
    mTextAlignment(Qt::AlignLeft | Qt::AlignVCenter),
    mRadius(6, 6),
    mSizeMode(Qt::AbsoluteSize),
    mHighlightGraph(nullptr)
{
    position->setType(QCPItemPosition::ptAbsolute);
    setSelectable(false);
    setLayer("overlay");

    setBorderPen(Qt::NoPen);
    setBrush(QColor(38, 41, 74));
    setTextColor(Qt::white);
    setOffset(15, -15);
    setPadding(QMargins(4, 4, 4, 4));
    connect(mParentPlot, SIGNAL(mouseMove(QMouseEvent*)), this, SLOT(handleTriggerEvent(QMouseEvent*)));
}

void QCPToolTip::setText(const QString& text)
{
    mText = text;
}

void QCPToolTip::setFont(const QFont& font)
{
    mFont = font;
}

void QCPToolTip::setTextColor(const QColor& color)
{
    mTextColor = color;
}

void QCPToolTip::setBorderPen(const QPen& pen)
{
    mBorderPen = pen;
}

void QCPToolTip::setBrush(const QBrush& brush)
{
    mBrush = brush;
}

void QCPToolTip::setRadius(double xRadius, double yRadius, Qt::SizeMode mode)
{
    mRadius = QPointF(QPoint(xRadius, yRadius));
    mSizeMode = mode;
}

void QCPToolTip::setOffset(double xOffset, double yOffset)
{
    mOffset = QPointF(QPoint(xOffset, yOffset));
}

void QCPToolTip::setPadding(const QMargins& paddings)
{
    mPadding = paddings;
}

//我们通过handleTriggerEvent来接收来自QCustomPlot的鼠标移动信号，作用是使得ToolTip跟随鼠标移动
void QCPToolTip::handleTriggerEvent(QMouseEvent* event)
{
    updatePosition(event->pos(), true);   // true 表示需要单独刷新，将调用update函数
}

void QCPToolTip::update()
{
    mPlotReplot = false;    // 表明单独刷新
    layer()->replot();
    mPlotReplot = true;    // 单独刷新完毕
}

// 不需要鼠标点击测试，因为ToolTip是跟随鼠标的，鼠标点击不到
double QCPToolTip::selectTest(const QPointF& pos, bool onlySelectable, QVariant* details) const
{
    Q_UNUSED(pos)
        Q_UNUSED(onlySelectable)
        Q_UNUSED(details)
        return -1;
}

//ToolTip的位置主要是遍历当前QCustomPlot的所有QCPGraph，找到鼠标下的对应的数据点
int QCPToolTip::pickClosest(double target, const QVector<double>& vector)
{
    if (vector.size() < 2)
        return 0;

    // 查找第一个大于或等于target的位置
    auto it = std::lower_bound(vector.constBegin(), vector.constEnd(), target);

    if (it == vector.constEnd()) return vector.size() - 1;
    else if (it == vector.constBegin()) return 0;
    else return target - *(it - 1) < *it - target ? (it - vector.constBegin() - 1) : (it - vector.constBegin());
}

void QCPToolTip::updatePosition(const QPointF& newPos, bool replot)
{
    mHighlightGraph = nullptr;
    double tolerance = mParentPlot->selectionTolerance();

    for (int i = mParentPlot->graphCount() - 1; i >= 0; --i) {
        QCPGraph* graph = mParentPlot->graph(i);
        if (!graph->realVisibility() || graph->scatterStyle().isNone())   // graph不可见或者scatter style 为空的时候，不显示ToolTip
            continue;

        double limitDistance = tolerance;   // limitDistance 用于选择的范围
        double penWidth = graph->pen().widthF();
        QCPScatterStyle scatterStyle = graph->scatterStyle();

        limitDistance = qMax(scatterStyle.size(), tolerance);
        penWidth = scatterStyle.isPenDefined() ? scatterStyle.pen().widthF() : penWidth;

        QVariant details;
        double currentDistance = graph->selectTest(newPos, false, &details);   // details会返回最接近的一个数据点，selectTest是不精确的，所以后面还要判断

        QCPDataSelection selection = details.value<QCPDataSelection>();
        if (currentDistance >= 0 && currentDistance < limitDistance + penWidth && !selection.isEmpty()) {
            // 取出当前key和value值，并且转换为像素位置
            double key = graph->dataMainKey(selection.dataRange().begin());
            double value = graph->dataMainValue(selection.dataRange().begin());

            QPointF pos = graph->coordsToPixels(key, value);

            QRectF rect(pos.x() - limitDistance * 0.5, pos.y() - limitDistance * 0.5, limitDistance, limitDistance);
            rect = rect.adjusted(-penWidth, -penWidth, penWidth, penWidth);

            if (rect.contains(newPos)) {    // 通过矩形判断，鼠标位置是否在数据点上
                // 解开以下注释，可以使得我们的文字跟轴标签的文字是一样的（但跟轴标签实际的显示效果可能是不一样的，这里要注意，例如对于科学计数法，轴可能会使用美化），同时要注意当轴标签不显示的时候tickVectorLabels返回的是空的，所以我们要做一下判断
                // 注意这里的方式是不精确的，适用于文字轴这种类型的
                int keyIndex = pickClosest(key, graph->keyAxis()->tickVector());
                setText(QString("当前: %1\n当前值: %2").arg(graph->keyAxis()->tickVectorLabels().at(keyIndex),
                    QString::number(value)));

                //setText(QString("本月份:%1\n当前值:%2").arg(QString::number(key), QString::number(value)));
                mHighlightGraph = graph;
                mGraphDataPos = pos;

                mParentPlot->setCursor(Qt::PointingHandCursor);
                position->setPixelPosition(newPos);  // 更新位置
                setVisible(true);

                if (replot) update();
                break;
            }
        }
    }

    if (!mHighlightGraph && visible()) {
        mParentPlot->setCursor(Qt::ArrowCursor);
        setVisible(false);
        if (replot) update();
    }
}

//绘制原理是在数据点上方重新绘制scatter style，并且稍微放大一点scatter style的大小，
//造成一种假象，此种方法适用于scatter style有一个背景画刷
void QCPToolTip::draw(QCPPainter* painter)
{
    if (mPlotReplot) {  // 当前是由QCustomPlot的replot函数刷新的，所以要更新位置
        updatePosition(position->pixelPosition(), false);  // 传入false表明不刷新
        if (!visible()) return;   // 由于位置更新之后，ToolTip可能会隐藏掉了，所以此处直接返回
    }

    drawGraphScatterPlot(painter, mHighlightGraph, mGraphDataPos);

    QPointF pos = position->pixelPosition() + mOffset;
    painter->translate(pos);  // 移动painter的绘制原点位置

    QFontMetrics fontMetrics(mFont);
    QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | mTextAlignment, mText);
    textRect.moveTopLeft(QPoint(mPadding.left(), mPadding.top()));

    QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
    textBoxRect.moveTopLeft(QPoint());

    // 限制ToolTip不超过QCustomPlot的范围
    if (pos.x() + textBoxRect.width() >= mParentPlot->viewport().right())
        painter->translate(-mOffset.x() * 2 - textBoxRect.width(), 0);
    if (pos.y() + textBoxRect.height() * 2 >= mParentPlot->viewport().bottom())
        painter->translate(0, -mOffset.y() * 2 - textBoxRect.height());

    // 绘制背景和边框
    if ((mBrush != Qt::NoBrush && mBrush.color().alpha() != 0) ||
        (mBorderPen != Qt::NoPen && mBorderPen.color().alpha() != 0)) {
        double clipPad = mBorderPen.widthF();
        QRect boundingRect = textBoxRect.adjusted(-clipPad, -clipPad, clipPad, clipPad);

        painter->setPen(mBorderPen);
        painter->setBrush(mBrush);
        painter->drawRoundedRect(boundingRect, mRadius.x(), mRadius.y(), mSizeMode);
    }

    // 绘制文字
    painter->setFont(mFont);
    painter->setPen(mTextColor);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(textRect, Qt::TextDontClip | mTextAlignment, mText);
}

void QCPToolTip::drawGraphScatterPlot(QCPPainter* painter, QCPGraph* graph, const QPointF& pos)
{
    if (!graph) return;
    QCPScatterStyle style = graph->scatterStyle();
    style.setBrush(Qt::red);           //设置圆心为红色
    if (style.isNone()) return;

    if (graph->selectionDecorator())   // 如果有select decorator，则使用修饰器的风格
        style = graph->selectionDecorator()->getFinalScatterStyle(style);

    style.applyTo(painter, graph->pen());
    style.setSize(style.size() * 1.2); // 放大一点
    style.drawShape(painter, pos);
}
