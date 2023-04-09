#include "CustomPlotTooltip.h"

//���ǽ�ToolTip����overlay�㣬ԭ������Ϊ��������ҪƵ����ˢ�£��������ǿ��Բ����»���ͼ���Ż��ٶȡ�
//ͬʱ��position��λ������Ϊ���صķ�ʽ
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

//����ͨ��handleTriggerEvent����������QCustomPlot������ƶ��źţ�������ʹ��ToolTip��������ƶ�
void QCPToolTip::handleTriggerEvent(QMouseEvent* event)
{
    updatePosition(event->pos(), true);   // true ��ʾ��Ҫ����ˢ�£�������update����
}

void QCPToolTip::update()
{
    mPlotReplot = false;    // ��������ˢ��
    layer()->replot();
    mPlotReplot = true;    // ����ˢ�����
}

// ����Ҫ��������ԣ���ΪToolTip�Ǹ������ģ����������
double QCPToolTip::selectTest(const QPointF& pos, bool onlySelectable, QVariant* details) const
{
    Q_UNUSED(pos)
        Q_UNUSED(onlySelectable)
        Q_UNUSED(details)
        return -1;
}

//ToolTip��λ����Ҫ�Ǳ�����ǰQCustomPlot������QCPGraph���ҵ�����µĶ�Ӧ�����ݵ�
int QCPToolTip::pickClosest(double target, const QVector<double>& vector)
{
    if (vector.size() < 2)
        return 0;

    // ���ҵ�һ�����ڻ����target��λ��
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
        if (!graph->realVisibility() || graph->scatterStyle().isNone())   // graph���ɼ�����scatter style Ϊ�յ�ʱ�򣬲���ʾToolTip
            continue;

        double limitDistance = tolerance;   // limitDistance ����ѡ��ķ�Χ
        double penWidth = graph->pen().widthF();
        QCPScatterStyle scatterStyle = graph->scatterStyle();

        limitDistance = qMax(scatterStyle.size(), tolerance);
        penWidth = scatterStyle.isPenDefined() ? scatterStyle.pen().widthF() : penWidth;

        QVariant details;
        double currentDistance = graph->selectTest(newPos, false, &details);   // details�᷵����ӽ���һ�����ݵ㣬selectTest�ǲ���ȷ�ģ����Ժ��滹Ҫ�ж�

        QCPDataSelection selection = details.value<QCPDataSelection>();
        if (currentDistance >= 0 && currentDistance < limitDistance + penWidth && !selection.isEmpty()) {
            // ȡ����ǰkey��valueֵ������ת��Ϊ����λ��
            double key = graph->dataMainKey(selection.dataRange().begin());
            double value = graph->dataMainValue(selection.dataRange().begin());

            QPointF pos = graph->coordsToPixels(key, value);

            QRectF rect(pos.x() - limitDistance * 0.5, pos.y() - limitDistance * 0.5, limitDistance, limitDistance);
            rect = rect.adjusted(-penWidth, -penWidth, penWidth, penWidth);

            if (rect.contains(newPos)) {    // ͨ�������жϣ����λ���Ƿ������ݵ���
                // �⿪����ע�ͣ�����ʹ�����ǵ����ָ����ǩ��������һ���ģ��������ǩʵ�ʵ���ʾЧ�������ǲ�һ���ģ�����Ҫע�⣬������ڿ�ѧ������������ܻ�ʹ����������ͬʱҪע�⵱���ǩ����ʾ��ʱ��tickVectorLabels���ص��ǿյģ���������Ҫ��һ���ж�
                // ע������ķ�ʽ�ǲ���ȷ�ģ��������������������͵�
                int keyIndex = pickClosest(key, graph->keyAxis()->tickVector());
                setText(QString("��ǰ: %1\n��ǰֵ: %2").arg(graph->keyAxis()->tickVectorLabels().at(keyIndex),
                    QString::number(value)));

                //setText(QString("���·�:%1\n��ǰֵ:%2").arg(QString::number(key), QString::number(value)));
                mHighlightGraph = graph;
                mGraphDataPos = pos;

                mParentPlot->setCursor(Qt::PointingHandCursor);
                position->setPixelPosition(newPos);  // ����λ��
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

//����ԭ���������ݵ��Ϸ����»���scatter style��������΢�Ŵ�һ��scatter style�Ĵ�С��
//���һ�ּ��󣬴��ַ���������scatter style��һ��������ˢ
void QCPToolTip::draw(QCPPainter* painter)
{
    if (mPlotReplot) {  // ��ǰ����QCustomPlot��replot����ˢ�µģ�����Ҫ����λ��
        updatePosition(position->pixelPosition(), false);  // ����false������ˢ��
        if (!visible()) return;   // ����λ�ø���֮��ToolTip���ܻ����ص��ˣ����Դ˴�ֱ�ӷ���
    }

    drawGraphScatterPlot(painter, mHighlightGraph, mGraphDataPos);

    QPointF pos = position->pixelPosition() + mOffset;
    painter->translate(pos);  // �ƶ�painter�Ļ���ԭ��λ��

    QFontMetrics fontMetrics(mFont);
    QRect textRect = fontMetrics.boundingRect(0, 0, 0, 0, Qt::TextDontClip | mTextAlignment, mText);
    textRect.moveTopLeft(QPoint(mPadding.left(), mPadding.top()));

    QRect textBoxRect = textRect.adjusted(-mPadding.left(), -mPadding.top(), mPadding.right(), mPadding.bottom());
    textBoxRect.moveTopLeft(QPoint());

    // ����ToolTip������QCustomPlot�ķ�Χ
    if (pos.x() + textBoxRect.width() >= mParentPlot->viewport().right())
        painter->translate(-mOffset.x() * 2 - textBoxRect.width(), 0);
    if (pos.y() + textBoxRect.height() * 2 >= mParentPlot->viewport().bottom())
        painter->translate(0, -mOffset.y() * 2 - textBoxRect.height());

    // ���Ʊ����ͱ߿�
    if ((mBrush != Qt::NoBrush && mBrush.color().alpha() != 0) ||
        (mBorderPen != Qt::NoPen && mBorderPen.color().alpha() != 0)) {
        double clipPad = mBorderPen.widthF();
        QRect boundingRect = textBoxRect.adjusted(-clipPad, -clipPad, clipPad, clipPad);

        painter->setPen(mBorderPen);
        painter->setBrush(mBrush);
        painter->drawRoundedRect(boundingRect, mRadius.x(), mRadius.y(), mSizeMode);
    }

    // ��������
    painter->setFont(mFont);
    painter->setPen(mTextColor);
    painter->setBrush(Qt::NoBrush);
    painter->drawText(textRect, Qt::TextDontClip | mTextAlignment, mText);
}

void QCPToolTip::drawGraphScatterPlot(QCPPainter* painter, QCPGraph* graph, const QPointF& pos)
{
    if (!graph) return;
    QCPScatterStyle style = graph->scatterStyle();
    style.setBrush(Qt::red);           //����Բ��Ϊ��ɫ
    if (style.isNone()) return;

    if (graph->selectionDecorator())   // �����select decorator����ʹ���������ķ��
        style = graph->selectionDecorator()->getFinalScatterStyle(style);

    style.applyTo(painter, graph->pen());
    style.setSize(style.size() * 1.2); // �Ŵ�һ��
    style.drawShape(painter, pos);
}
