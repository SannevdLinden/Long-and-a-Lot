#include "scatter_lines.h"
#include "qgraphicssceneevent.h"
#include <QPainter>

scatter_lines::scatter_lines(int w, int h, QGraphicsItem* parent) :
    QGraphicsObject(parent)
{
    _h = h;
    _w = w;
}

void scatter_lines::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    QPen pen;  // creates a default pen
    pen.setStyle(Qt::NoPen);
    painter->setPen(pen);
    painter->setBrush(QBrush(QColor(48,48,48)));
    painter->drawRect(0,0,_w,_h);
    pen.setStyle(Qt::SolidLine);
    painter->setPen(pen);
    painter->setPen(QPen(QColor(255,255,255), 0));
    painter->setBrush(QBrush(QColor(255,255,255)));
    painter->drawLine(0,_h,_w,_h);
    painter->drawLine(0,0,0,_h);

}

QRectF scatter_lines::boundingRect() const
{
   return QRectF(0,0, _w, _h);
}


