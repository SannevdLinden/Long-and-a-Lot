#ifndef TREE_H
#define TREE_H

#include <QGraphicsObject>
#include <QObject>

class tree : public QGraphicsObject
{
    Q_OBJECT
public:
    tree(qreal width_rect, qreal height_rect, QString name, bool hover, int offset_y, bool overflow_left,
         QColor color, QMap<QString, double> total_freqs, QMap<QString, QColor> c,
         QList<QString> high, bool overflow_right, QString mode,
         QMap<QString, QList<QString>> pat_id, QList<QString> most_freq_pat,
         QList<QString> filtered_out_cats, QGraphicsItem* parent =0);

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr);
    QRectF boundingRect() const;
    void setIndex(QVector<double> return_val);
    QVector<double> getIndex();
    void overflow_left(bool overflow);
    void overflow_right(bool overflow);

public slots:
    void setScaleXY(QVariant val);    

private:
    int _x,_y, _offset_y;
    qreal _width_rect, _height_rect;
    QString _name;
    bool _hover, _overflow_left, _overflow_right;
    QVector<double> _zoom, _return_val;
    QColor _color;
    QMap<QString, double> _total_freqs;
    QMap<QString, QColor> _c;
    QList<QString> _high;
    QString _mode;
    QMap<QString, QList<QString>> _pat_id;
    QList<QString> _most_freq_pat, _filtered_out_cats;


protected:
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);

signals:
    void stageRectClicked(QVector<double> stage_rect_clicked);
    void stageZoom(QVector<double> stage_zoom);
};



#endif // TREE_H


