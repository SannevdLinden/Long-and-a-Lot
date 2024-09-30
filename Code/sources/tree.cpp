#include "tree.h"
#include <QPainter>
#include <QObject>
#include <QWidget>
#include <QGraphicsSceneWheelEvent>

tree::tree(qreal width_rect, qreal height_rect, QString name, bool hover, int offset_y,
           bool overflow_left, QColor color, QMap<QString, double> total_freqs,
           QMap<QString, QColor> c, QList<QString> high, bool overflow_right,
           QString mode, QMap<QString, QList<QString>> pat_id, QList<QString> most_freq_pat,
           QList<QString> filtered_out_cats, QGraphicsItem* parent):
    QGraphicsObject(parent)
{
    _width_rect = width_rect;
    _height_rect = height_rect;
    _x = 0;
    _y = 0;
    _name = name;
    _zoom.resize(5);
    _return_val.resize(2);
    _hover = hover;
    _offset_y = offset_y;
    _overflow_left = overflow_left;
    _overflow_right = overflow_right;
    _color = color;
    _total_freqs = total_freqs;
    _c = c;
    _high = high;
    _mode = mode;
    _pat_id = pat_id;
    _most_freq_pat = most_freq_pat;
    _filtered_out_cats = filtered_out_cats;
}

void tree::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setRenderHint(QPainter::Antialiasing);
    if(_overflow_left){
        painter->setBrush(QBrush(QColor(128,128,128)));
        painter->setPen(QPen(QColor(128,128,128), 3));
        painter->drawLine(5,(_height_rect-_offset_y)*0.5+_offset_y,8,(_height_rect-_offset_y)*0.5-5+_offset_y);
        painter->drawLine(5,(_height_rect-_offset_y)*0.5+_offset_y,8,(_height_rect-_offset_y)*0.5+5+_offset_y);
    }
    if(_overflow_right){
        painter->setBrush(QBrush(QColor(128,128,128)));
        painter->setPen(QPen(QColor(128,128,128), 3));
        painter->drawLine(_width_rect-8,(_height_rect-_offset_y)*0.5-5+_offset_y,_width_rect-5,(_height_rect-_offset_y)*0.5+_offset_y);
        painter->drawLine(_width_rect-5,(_height_rect-_offset_y)*0.5+_offset_y,_width_rect-8,(_height_rect-_offset_y)*0.5+5+_offset_y);
    }
    if(_name == ""){
        painter->setPen(QPen(QColor(255,255,255, 100), 0));
        if(_height_rect > 10){
            QRectF rsmall(_x, 0, _width_rect, _offset_y+17);
            painter->drawRect(rsmall);
         }
    } else {
       painter->setPen(Qt::NoPen);
        //painter->setPen(QPen(QColor(255,255,255, 50), 0));
    }
    if(_hover){
        painter->setBrush(QBrush(QColor(104,104,104)));
    } else {
        painter->setBrush(QBrush(QColor(48,48,48)));
    }    
    if(_height_rect <= 10 || _hover){
        QRectF r(_x,_y,_width_rect, _height_rect);
        painter->drawRect(r);
    }
    if(_name != "") {        
        painter->setBrush(QBrush(_color));
        painter->setPen(Qt::NoPen);
        QRectF r2(_x,_y,_width_rect, _offset_y+17);
        painter->drawRect(r2);
    }

    painter->setPen(QPen(QColor(255,255,255), 0));
    painter->setBrush(QBrush(QColor(48,48,48)));
    if(_hover){
        painter->setBrush(QBrush(QColor(104,104,104)));
    }
    if(_name != "") {
        const QRectF rect_name = QRectF(_x, _offset_y, _width_rect, 17);
        painter->drawText(rect_name, Qt::AlignCenter, _name);
    } else {
        //draw aggregated stacked bar chart
        if(_mode == "patterns"){
            int num_cats = _total_freqs.size();
            if(num_cats > _most_freq_pat.size()){
                num_cats = _most_freq_pat.size();
            }

            if(_width_rect >= num_cats*20 && _height_rect <= 10){
                painter->setPen(Qt::NoPen);
                QList<QString> labels;
                labels.resize(_total_freqs.size());
                int ind = 0;
                int total = 0;
                int not_zero = 0;
                for (auto i = _total_freqs.cbegin(), end = _total_freqs.cend(); i != end; ++i)
                {
                     if(_most_freq_pat.contains(i.key())){
                          labels[ind] = i.key();
                          ind ++;
                          total += i.value();
                          if(i.value() > 0){
                              not_zero ++;
                          }
                     }
                }
                double pixel_per_one = (1.0*_width_rect - (not_zero-1)*2.0)/(1.0*total);
                double x = 0;

                for (int i = 0; i < ind; i++){
                    if(_total_freqs[labels[i]] > 0){
                        int val = _total_freqs[labels[i]];
                        qreal val_color = (val*1.0*pixel_per_one) / (_pat_id[labels[i]].size()*1.0); //width of each event in current pat
                        qreal prev_x = 0.0;
                        for(int j= 0; j< _pat_id[labels[i]].size(); j++){
                            QColor col = _c[_pat_id[labels[i]][j]];
                            if(_high.size() ==1){
                                if(!_high.contains(labels[i])){
                                   col = QColor(col.red(), col.green(), col.blue(), 50);
                               }
                            }
                            if(_filtered_out_cats.contains(_pat_id[labels[i]][j])){
                              col = QColor(col.red(), col.green(), col.blue(), 50);
                            }
                            painter->setBrush(QBrush(col));
                            QRectF r(x+prev_x,0, val_color, _height_rect);
                            painter->drawRect(r);
                            prev_x += val_color;
                        }
                        x += _total_freqs[labels[i]]*pixel_per_one + 2;
                    }
                }
            } else if(_width_rect < num_cats*20 && _width_rect > 5 && _height_rect <= 10){
                //pick top x patterns to show each event in pattern with 1 pixel + indicator
                int num_cat = 0;
                QList<QString> labels;
                int total = 0;
                int not_zero = 0;
                int pixels_left = _width_rect;
                QList<QPair<int, QString>> tmp;
                for(int i=0; i < _most_freq_pat.size(); i++){
                    tmp.append(qMakePair(_total_freqs[_most_freq_pat[i]],_most_freq_pat[i]));
                }
                std::sort(std::begin(tmp), std::end(tmp));
                for(int i=tmp.size()-1; i >= 0; i--){
                   pixels_left -= _pat_id[tmp[i].second].size() * 2 + 1;
                   if(pixels_left > 0){
                       num_cat ++;
                       labels.append(tmp[i].second);
                       total += tmp[i].first;
                       if(_total_freqs[tmp[i].second] > 0){
                           not_zero ++;
                       }
                   }
                }
                painter->setPen(Qt::NoPen);
                double pixel_per_one = (1.0*_width_rect - (not_zero-1)*1.0)/(1.0*total);
                double x = 0;
                for (int i = 0; i < num_cat; i++){
                    if(_total_freqs[labels[i]] > 0){
                        int val = _total_freqs[labels[i]];
                        qreal val_color = (val*1.0*pixel_per_one) / (_pat_id[labels[i]].size()*1.0); //width of each event in current pat
                        qreal prev_x = 0.0;
                        for(int j= 0; j< _pat_id[labels[i]].size(); j++){
                            QColor col = _c[_pat_id[labels[i]][j]];
                            if(_high.size() ==1){
                                if(!_high.contains(labels[i])){
                                   col = QColor(col.red(), col.green(), col.blue(), 50);
                               }
                            }
                            if(_filtered_out_cats.contains(_pat_id[labels[i]][j])){
                              col = QColor(col.red(), col.green(), col.blue(), 50);
                            }
                            painter->setBrush(QBrush(col));
                            QRectF r(x+prev_x,0, val_color, _height_rect);
                            painter->drawRect(r);
                            prev_x += val_color;
                        }
                        x += _total_freqs[labels[i]]*pixel_per_one + 1;
                    }
                }

                if(num_cat < _total_freqs.size()){
                    QRectF rect = QRectF(_width_rect-5, _height_rect-5, 5, 5);
                    QPainterPath path;
                    path.moveTo(rect.topRight());
                    path.lineTo(rect.bottomLeft());
                    path.lineTo(rect.bottomRight());
                    path.lineTo(rect.topRight());

                    painter->fillPath(path, QBrush(QColor ("red")));
                }
            }
        } else {
            int num_cats = _total_freqs.size();

            if(_width_rect >= num_cats && _height_rect <= 10){
                painter->setPen(Qt::NoPen);
                int total = 0;
                for (auto i = _total_freqs.cbegin(), end = _total_freqs.cend(); i != end; ++i){
                    total += i.value();
                }
                double pixel_per_one = (1.0*_width_rect)/(1.0*total);
                double x = 0;

                for (auto i = _total_freqs.cbegin(), end = _total_freqs.cend(); i != end; ++i){
                    QColor col = _c[i.key()];
                    if(_high.size() ==1){
                        if(!_high.contains(i.key())){
                           col = QColor(col.red(), col.green(), col.blue(), 50);
                       }
                    }
                    painter->setBrush(QBrush(col));
                    double w_rect = i.value()*pixel_per_one;
                    if (w_rect < 1 && i.value() > 0){
                        w_rect = 1;
                    }
                    QRectF r(x,0, w_rect, _height_rect);
                    painter->drawRect(r);
                    x += i.value()*pixel_per_one;
                }
            } else if(_width_rect < num_cats && _width_rect > 5 && _height_rect <= 10){
                //pick top 3 cats to show each 1 pixel + indicator
                int num_cat = floor(_width_rect/2);
                QList<double> tmp = _total_freqs.values();
                tmp = tmp.mid(0,num_cat);
                painter->setPen(Qt::NoPen);
                int total = 0;
                for (auto i = _total_freqs.cbegin(), end = _total_freqs.cend(); i != end; ++i){
                    if(tmp.contains(i.value())){
                        total += i.value();
                    }
                }
                double pixel_per_one = (1.0*_width_rect)/(1.0*total);
                double x = 0;
                qreal h = _height_rect;

                for (auto i = _total_freqs.cbegin(), end = _total_freqs.cend(); i != end; ++i){
                    if(tmp.contains(i.value())){
                        QColor col = _c[i.key()];
                        if(_high.size() ==1){
                            if(!_high.contains(i.key())){
                               col = QColor(col.red(), col.green(), col.blue(), 50);
                           }
                        }
                        painter->setBrush(QBrush(col));
                        double w_rect = i.value()*pixel_per_one;
                        QRectF r(x,0, w_rect, h);
                        painter->drawRect(r);
                        x += i.value()*pixel_per_one;
                    }
                }
                if(num_cat < _total_freqs.size()){
                    QRectF rect = QRectF(_width_rect-5, _height_rect-5, 5, 5);
                    QPainterPath path;
                    path.moveTo(rect.topRight());
                    path.lineTo(rect.bottomLeft());
                    path.lineTo(rect.bottomRight());
                    path.lineTo(rect.topRight());

                    painter->fillPath(path, QBrush(QColor ("red")));
                }

            }
         }
    }

}


QRectF tree::boundingRect() const
{
   return QRectF(0,0, _width_rect, _height_rect);
}

void tree::setIndex(QVector<double> return_val){
    _return_val = return_val;
}

QVector<double> tree::getIndex(){
   return _return_val;
}


void tree::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
    if (mouseEvent->button() == Qt::LeftButton &&
            mouseEvent->modifiers() == Qt::ControlModifier){
           emit stageRectClicked(_return_val);
    }
    QGraphicsObject::mousePressEvent(mouseEvent);
}


void tree::setScaleXY(QVariant val)
{
    QTransform transform2;
    transform2.scale(val.toPointF().x(),val.toPointF().y());
    this->setTransform(transform2);
}

void tree::overflow_left(bool overflow){
    _overflow_left = overflow;
    this->update();
}

void tree::overflow_right(bool overflow){
    _overflow_right = overflow;
    this->update();
}
