#ifndef SCATTER_LINES_H
#define SCATTER_LINES_H

#include <QGraphicsObject>
#include <QObject>

class scatter_lines : public QGraphicsObject
{
    Q_OBJECT

public:
    scatter_lines(int w, int h,  QGraphicsItem* parent =0);
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
     QRectF boundingRect() const;

private:
    int _w, _h;



};

#endif // SCATTER_LINES_H
