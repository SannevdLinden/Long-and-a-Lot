#ifndef DOTS_H
#define DOTS_H

#include <QGraphicsObject>
#include <QObject>

class dots : public QGraphicsObject
{
    Q_OBJECT
public:
    dots(int w, QColor c, int id,QGraphicsItem* parent =0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    QRectF boundingRect() const;
    int getIndex();
    void high_after_box_sel(bool high);

private:
    int _w, _dot_id;
    QColor _color;
    bool _high = false;

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event);



signals:
    void dotClicked(int dot_id);
    void dotHoverTrue(int dot_id);
    void dotHoverFalse(int dot_id);

};

#endif // DOTS_H
