#ifndef GRAPHICSVIEWZOOM_H
#define GRAPHICSVIEWZOOM_H

#include <QGraphicsView>
#include <QObject>

class GraphicsViewZoom : public QGraphicsView
{
    Q_OBJECT
public:
    GraphicsViewZoom(QWidget *parent);

protected:
    void wheelEvent(QWheelEvent *event);
    void mousePressEvent(QMouseEvent *mouseEvent);
    void mouseReleaseEvent(QMouseEvent *mouseEvent);

signals:
    void boxSelection(bool selection);
};

#endif // GRAPHICSVIEWZOOM_H
