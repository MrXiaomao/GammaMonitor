#include "mytracer.h"

myTracer::myTracer(QCustomPlot* _plot, QCPGraph* _graph, TracerType _type) :
    plot(_plot), graph(_graph), type(_type), visible(false)
{
    if (plot) {
        tracer = new QCPItemTracer(plot);
        //tracer->setStyle(QCPItemTracer::tsCircle);//追踪光标样式，这是小十字，还有大十字、园等
        //tracer->setPen(QPen(Qt::red));
        tracer->setPen(graph->pen());   //设置tracer颜色
        tracer->setBrush(graph->pen().color());
        tracer->setSize(10);

        //label = new QCPItemText(plot);
        label = new QCPItemTip(plot);
        label->setRectColor(QColor("#00ffff")); //文本框颜色
        label->setLayer("overlay");
        label->setClipAxisRect(false);
        label->setPadding(QMargins(5, 5, 5, 5));
        label->position->setParentAnchor(tracer->position);
        label->setColor(QColor(Qt::black));

        arrow = new QCPItemLine(plot);
        arrow->setLayer("overlay");
        arrow->setPen(graph->pen());//设置箭头颜色
        arrow->setClipAxisRect(false);
        arrow->setHead(QCPLineEnding::esSpikeArrow);

        switch (type) {
        case XAxisTracer: {
            tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
            tracer->position->setTypeY(QCPItemPosition::ptAxisRectRatio);

            label->setPositionAlignment(Qt::AlignTop | Qt::AlignHCenter);
            //label->setBrush(QBrush(QColor(0,0,0,0)));
            label->setPen(QPen(QColor(0, 0, 0, 0)));


            arrow->end->setParentAnchor(tracer->position);
            arrow->start->setParentAnchor(arrow->end);
            arrow->start->setCoords(20, 0);//偏移量
            break;
        }

        case YAxisTracer: {
            tracer->position->setType(QCPItemPosition::ptAxisRectRatio);
            tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);

            label->setBrush(QBrush(QColor(244, 244, 244, 100)));
            label->setPen(QPen(Qt::black));
            label->setPositionAlignment(Qt::AlignRight | Qt::AlignHCenter);

            arrow->end->setParentAnchor(tracer->position);
            arrow->start->setParentAnchor(label->position);
            arrow->start->setCoords(-20, 0);//偏移量
            break;
        }

        case DataTracer: {
            tracer->position->setTypeX(QCPItemPosition::ptPlotCoords);
            tracer->position->setTypeY(QCPItemPosition::ptPlotCoords);

            label->setBrush(QBrush(QColor(244, 244, 244, 150)));
            label->setPen(graph->pen());
            label->setPositionAlignment(Qt::AlignLeft | Qt::AlignVCenter); 

            arrow->end->setParentAnchor(tracer->position);
            //arrow->start->setParentAnchor(arrow->end);
            arrow->start->setParentAnchor(label->top);  //设置该直线的起点为文字框的上锚点
            //arrow->start->setCoords(25, 0); //偏移量
            break;
        }
        default:
            break;
        }
        setVisible(false);
    }
}

myTracer::~myTracer() {
    if (tracer) {
        plot->removeItem(tracer);
    }
    if (label) {
        plot->removeItem(label);
    }
    if (arrow){
        plot->removeItem(arrow);
    }
}

void myTracer::setPen(const QPen& pen) {
    tracer->setPen(pen);
    arrow->setPen(pen);
}

void myTracer::setBrush(const QBrush& brush) {
    tracer->setBrush(brush);
}

void myTracer::setLabelPen(const QPen& pen) {
    label->setPen(pen);
}

void myTracer::setText(const QString& text) {
    label->setText(text);
}

void myTracer::setVisible(bool visible) {
    tracer->setVisible(visible);
    arrow->setVisible(visible);
    label->setVisible(visible);
}

void myTracer::updatePosition(double xValue, double yValue) {
    if (!visible) {
        setVisible(true);
        visible = true;
    }
    if (yValue > plot->yAxis->range().upper) {
        yValue = plot->yAxis->range().upper;
    }
    switch (type) {
    case XAxisTracer: {
        tracer->position->setCoords(xValue, 0);  //0:上x轴，1:下x轴
        label->position->setCoords(0, -25);
        //arrow->start->setCoords(0,15);
        //arrow->end->setCoords(0,0);
        break;
    }
    case YAxisTracer: {
        tracer->position->setCoords(1, yValue);
        label->position->setCoords(-20, 0);
        break;

    }
    case DataTracer: {
        tracer->position->setCoords(xValue, yValue);
        label->position->setType(QCPItemPosition::ptAbsolute);//位置类型
        label->position->setCoords(-70, 25);
        /*
    ptAbsolute        ///< Static positioning in pixels, starting from the top left corner of the viewport/widget.
    , ptViewportRatio  ///< Static positioning given by a fraction of the viewport size. For example, if you call setCoords(0, 0), the position will be at the top
                      ///< left corner of the viewport/widget. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and
                      ///< vertically at the top of the viewport/widget, etc.
    , ptAxisRectRatio  ///< Static positioning given by a fraction of the axis rect size (see \ref setAxisRect). For example, if you call setCoords(0, 0), the position will be at the top
                      ///< left corner of the axis rect. setCoords(1, 1) will be at the bottom right corner, setCoords(0.5, 0) will be horizontally centered and
                      ///< vertically at the top of the axis rect, etc. You can also go beyond the axis rect by providing negative coordinates or coordinates larger than 1.
    , ptPlotCoords     ///< Dynamic positioning at a plot coordinate defined by two axes (see \ref setAxes).
*/
        break;
    }
    default:
        break;
    }
}
