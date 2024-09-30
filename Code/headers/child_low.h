#ifndef CHILD_LOW_H
#define CHILD_LOW_H

#include <QGraphicsObject>
#include <QObject>

class child_low: public QGraphicsObject
{
    Q_OBJECT

public:
    child_low(int width, int height, QString cat, int ind, QMap<QString,
              QColor> c, QList<QString> high, bool high_hover_cluster, QString parent,
              bool non_high_color_pat, bool pat_high);
    void update_hover();

private:
    int _width, _height, _ind;
    QString _cat;
    QMap<QString, QColor> _c;
    QList<QString> _high;
    bool _high_hover_cluster, _non_high_color_pat, _pat_high;
    QString _ind_parent;

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QRectF boundingRect() const;
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);
    void hoverEnterEvent(QGraphicsSceneHoverEvent *event);
    void hoverLeaveEvent(QGraphicsSceneHoverEvent *event);

signals:
    void rectEventClicked(QString rect_clicked);
    void seqHigh(int ind);
    void seqLow(int ind);
};

#endif // CHILD_LOW_H
