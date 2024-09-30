#include "parent_low.h"
#include <QPainter>
#include <QGraphicsSceneWheelEvent>

parent_low::parent_low(int width, int height, bool hover, QString name, QString mode)
{
    _width = width;
    _height = height;
    _hover = hover;
    _w = width;
    _h = height;
    _name = name;
    _prev_pos.setX(0);
    _prev_pos.setY(0);
    _mode_stage = mode;
    this->setFlag(QGraphicsItem::ItemIsMovable);
    this->setAcceptHoverEvents(true);
}

void parent_low::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QColor(48,48,48));
    if(_hover){
        painter->setBrush(QBrush(QColor(104,104,104)));
    }
    painter->drawRect(0, 0, _width, _height);

}

void parent_low::set_bounding(int w, int h){
    _w = w;
    _h = h;
}

QRectF parent_low::boundingRect() const
{
    return QRectF(-_w, -_h, _w*2, _h*2);
}

void parent_low::wheelEvent(QGraphicsSceneWheelEvent* event)
{     
    if(_mode_stage == "mid"){
        double zoom_level;
        if (event->delta() > 0)
        {
            zoom_level = -0.15;
        }
        else
        {
            zoom_level = 0.15;
        }
        emit zoom_mid_level(qMakePair(zoom_level, _name));

    } else {
        if (event->delta() > 0)
        {
            this->setScale(this->scale() + 0.15);
        }
        else
        {
            this->setScale(this->scale() - 0.15);
        }
        emit zoom(qMakePair(this->scale(), _name));
    }

}

void parent_low::mouseMoveEvent(QGraphicsSceneMouseEvent *event){
    if(abs(this->pos().x() - _prev_pos.x()) > 3 || abs(this->pos().y() - _prev_pos.y()) > 3){
        emit pan(qMakePair(this->pos(), _name));
    }
    QGraphicsObject::mouseMoveEvent(event);
}

void parent_low::setScaleFunc(double scale){
    this->setScale(scale);
}

void parent_low::setPosFunc(QPointF pan){
    this->setPos(pan);
}

