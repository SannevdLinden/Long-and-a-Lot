#ifndef GRANDPARENT_H
#define GRANDPARENT_H

#include <QGraphicsObject>
#include <QObject>
#include <QPen>
#include <QBrush>

class grandParent: public QGraphicsObject
{
    Q_OBJECT
public:
    grandParent(qreal width, qreal height, bool hover);

private:
    qreal _width, _height;
    bool _hover;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QRectF boundingRect() const;

    void wheelEvent(QGraphicsSceneWheelEvent* event);

};

#endif // GRANDPARENT_H



