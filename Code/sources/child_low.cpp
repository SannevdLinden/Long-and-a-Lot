#include "child_low.h"
#include "qgraphicssceneevent.h"
#include <QPainter>

child_low::child_low(int width, int height, QString cat, int ind,
                     QMap<QString, QColor> c, QList<QString> high,
                     bool high_hover_cluster,  QString parent, bool non_high_color_pat,
                     bool pat_high)
{
    _width = width;
    _height = height;
    _cat = cat;
    _ind = ind;
    _c = c;
    _high = high;
    _high_hover_cluster = high_hover_cluster;
    _ind_parent = parent;
    _non_high_color_pat = non_high_color_pat;
    _pat_high = pat_high;
    this->setAcceptHoverEvents(true);
}

void child_low::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    bool option1 = false;
    QColor col = _c[_cat];
    if(_high.size() >=1 && !_pat_high){
        if(!_high.contains(_cat)){
           col = QColor(col.red(), col.green(), col.blue(), 50);           
       }
       option1 = true;
    }
    if(!_high_hover_cluster && !option1){
        col = QColor(col.red(), col.green(), col.blue(), 50);
    }
    if(_non_high_color_pat && _pat_high){
        col = QColor(col.red(), col.green(), col.blue(), 50);
    }
    painter->setBrush(QBrush(col));

    painter->setPen(Qt::NoPen);
    painter->drawRect(0, 0, _width, _height);

}

QRectF child_low::boundingRect() const
{
    return QRectF(0, 0, _width, _height);
}

void child_low::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
     emit rectEventClicked(_cat + "/" + QString::number(mouseEvent->scenePos().x())
                           + "/" + QString::number(mouseEvent->scenePos().y()) + "/" +
                           _ind_parent);

}

void child_low::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
    emit seqHigh(_ind);
}

void child_low::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
    emit seqLow(_ind);
}

void child_low::update_hover(){
    _high_hover_cluster = true;
    update();
}
