#ifndef PARENT_LOW_H
#define PARENT_LOW_H

#include <QGraphicsObject>
#include <QObject>
#include <QPen>
#include <QBrush>

class parent_low: public QGraphicsObject
{
    Q_OBJECT
public:
    parent_low(int width, int height, bool hover, QString name, QString mode);
    void set_bounding(int w, int h);
    void setScaleFunc(double scale);
    void setPosFunc(QPointF pan);

private:
    int _width, _height, _w, _h;
    bool _hover;
    QString _name;
    QPointF _prev_pos;
    QString _mode_stage;


protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QRectF boundingRect() const;
    void wheelEvent(QGraphicsSceneWheelEvent* event);
    void mouseMoveEvent(QGraphicsSceneMouseEvent *event);

signals:
    void zoom(QPair<double, QString> scale);
    void zoom_mid_level(QPair<double, QString> scale);
    void pan(QPair<QPointF, QString> pan_val);

};


#endif // PARENT_LOW_H
