#include "dots.h"
#include <QPainter>

dots::dots(int w, QColor c, int id, QGraphicsItem* parent):
    QGraphicsObject(parent)
{
    setAcceptHoverEvents(true);
    _w = w;
    _color = c;
    _dot_id = id;
}

void dots::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    painter->setBrush(QBrush(_color));
    painter->setPen(QPen(QColor(128,128,128,175), 0));
    painter->drawEllipse(0,0, _w, _w);

}

int dots::getIndex(){
   return _dot_id;
}


QRectF dots::boundingRect() const
{
   return QRectF(0,0, _w, _w);
}

void dots::high_after_box_sel(bool high){
    _high = high;
    QTransform transform;
    if(_high){
        transform.scale(2,2);
        this->setTransform(transform);        
    } else {
        transform.scale(1,1);
        this->setTransform(transform);        
    }
}

void dots::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
        QTransform transform;
        if(_high){
            transform.scale(1,1);
            this->setTransform(transform);
            _high = false;
        } else {
            transform.scale(2,2);
            this->setTransform(transform);
            _high = true;
        }
        emit dotClicked(_dot_id);
}

void dots::hoverEnterEvent(QGraphicsSceneHoverEvent* event){
        emit dotHoverTrue(_dot_id);

}

void dots::hoverLeaveEvent(QGraphicsSceneHoverEvent* event){
        emit dotHoverFalse(_dot_id);

}
