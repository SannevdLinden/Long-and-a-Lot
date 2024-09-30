#ifndef BAR_CHART_HIGH_H
#define BAR_CHART_HIGH_H

#include <QGraphicsObject>
#include <QObject>

class bar_chart_high: public QGraphicsObject
{
    Q_OBJECT
public:
    bar_chart_high(qreal width_rect, qreal height_rect, QString name, QMap<QString, double> freq_per_stage,
                   int _max_val, bool hover, QMap<QString, QColor> c, QList<QString> high, QString mode,
                   QMap<QString, QList<QString>> pat_id, QList<QString> most_freq_pat,
                   QList<QString> filtered_out_cats, int max_total_val, bool norm);

private:
    int _x,_y, _max_val, _max_tick_y, _max_total_val;
    qreal _width_rect, _height_rect;
     QString _name;
    QMap<QString, double> _freq_per_stage;
    bool _hover, _norm;
    QMap<QString, QColor> _c;
    QMap<QString, QList<double>> _pos_rect;
    QList<QString> _high;
    QString _mode;
    QMap<QString, QList<QString>> _pat_id;
    QList<QString> _most_freq_pat, _filtered_out_cats;


protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget);
    QRectF boundingRect() const;
    void wheelEvent(QGraphicsSceneWheelEvent* event);
    void mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent);


signals:
    void rectBarClicked(QString rect_clicked);
};

#endif // BAR_CHART_HIGH_H
