#include "grandparent.h"
#include <QPainter>
#include <QGraphicsSceneWheelEvent>

grandParent::grandParent(qreal width, qreal height, bool hover)
{
    _width = width;
    _height = height;
    _hover = hover;
}

void grandParent::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(QColor(48,48,48)));
    if(_hover){
       painter->setBrush(QBrush(QColor(104,104,104)));
    }
    QRectF r(0, 0, _width, _height);
    painter->drawRect(r);

}

QRectF grandParent::boundingRect() const
{
    return QRectF(0, 0, _width, _height);
}

void grandParent::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    Q_UNUSED(event);
    // For the parent item, do nothing if scrolled on

}
