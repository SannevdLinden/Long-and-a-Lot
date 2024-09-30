#include "bar_chart_high.h"
#include <QPainter>
#include <QGraphicsSceneWheelEvent>

bar_chart_high::bar_chart_high(qreal width_rect, qreal height_rect, QString name, QMap<QString, double> freq_per_stage,
                               int max_val, bool hover, QMap<QString, QColor> c, QList<QString> high,
                               QString mode, QMap<QString, QList<QString>> pat_id,
                               QList<QString> most_freq_pat, QList<QString> filtered_out_cats, int max_total_val, bool norm)
{
    _width_rect = width_rect;
    _height_rect = height_rect;
    _freq_per_stage = freq_per_stage;
    _x = 0;
    _y = 0;
    _name = name;
    _max_val = max_val;
    _max_tick_y = 5;
    _hover = hover;
    _c = c;
    _high = high;
    _mode = mode;
    _pat_id = pat_id;
    _most_freq_pat = most_freq_pat;
    _filtered_out_cats = filtered_out_cats;
    _max_total_val = max_total_val;
    _norm = norm;
    this->setFlag(QGraphicsItem::ItemIsMovable);
}

void bar_chart_high::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);
    int space_for_ticks = 25;
    qreal w = _width_rect;
    qreal h = _height_rect;
    int offset = 20;

    painter->setRenderHint(QPainter::Antialiasing);    
    if(_hover){
        painter->setBrush(QBrush(QColor(104,104,104)));
        painter->setPen(Qt::NoPen);
        QRectF r(0,0,w,h);
        painter->drawRect(r);
    }
    painter->setPen(QPen(QColor(255,255,255), 0));
    painter->setBrush(QBrush(QColor(255,255,255)));
    QList<QString> labels;
    labels.resize(_freq_per_stage.size());
    int ind = 0;
    int total = 0;
    int not_zero = 0;
    int highest_val = 0;
    for (auto i = _freq_per_stage.cbegin(), end = _freq_per_stage.cend(); i != end; ++i)
    {
      if(_mode == "patterns"){
          if(_most_freq_pat.contains(i.key())){
              labels[ind] = i.key();
              ind ++;
              total += i.value();
              if(i.value() > 0){
                  not_zero ++;
              }
              if(i.value() > highest_val){
                  highest_val = i.value();
              }
          }
      } else {
          labels[ind] = i.key();
          ind ++;
          total += i.value();
          if(i.value() > highest_val){
              highest_val = i.value();
          }
      }

    }

    if(_mode == "patterns"){
        if(ind > 20){
            ind = 20;
        }
        if(_width_rect > 100){
            painter->setPen(QPen(QColor(255,255,255), 0));
            painter->setBrush(QBrush(QColor(255,255,255)));
            //x-axis
            painter->drawLine(_x + space_for_ticks + offset, h - space_for_ticks, _x + w - space_for_ticks, h - space_for_ticks);
            //y-axis
            painter->drawLine(_x + space_for_ticks +offset, h  - space_for_ticks , _x + space_for_ticks + offset, 0);
            //ticks y-axis
            if(_norm){
                for (int i = 0; i < 6; i++)
                {
                    const QRect rect_first = QRect(_x +2, h  - 1.5*space_for_ticks -2 - i*((h - space_for_ticks-10)/5.0),
                                                   space_for_ticks, space_for_ticks);
                    painter->drawText(rect_first, Qt::AlignLeft, QString::number(i*2) + "0");
                    painter->drawLine(_x + space_for_ticks -4 +offset, h - space_for_ticks - i*((h - space_for_ticks)/5.0),
                                      _x + space_for_ticks + offset, h - space_for_ticks - i*((h - space_for_ticks)/5.0));
                }
            } else {
                //first and last tick
                const QRect rect_first = QRect(_x +2, h  - 1.5*space_for_ticks -2 , space_for_ticks, space_for_ticks);
                painter->drawText(rect_first, Qt::AlignLeft, "0");
                painter->drawLine(_x + space_for_ticks -4 +offset, h - space_for_ticks, _x + space_for_ticks + offset, h - space_for_ticks);

                QString tmp_text = QString::number(_max_val);
                if(tmp_text.size() >= 5){
                    tmp_text = tmp_text.mid(0,tmp_text.size()-3) + "k";
                }
                const QRect rect_last = QRect(_x +2, 0, space_for_ticks, space_for_ticks);
                painter->drawText(rect_last, Qt::AlignLeft, tmp_text);
                painter->drawLine(_x + space_for_ticks-4 +offset, 0, _x + space_for_ticks +offset, 0 );

                //the other ticks
                if(_max_tick_y - 2 > 0)
                {
                    int pixel_per_label_y = floor((h - space_for_ticks)/(_max_tick_y -1)); //0 tick not included
                    float tick_label = float(_max_val)/(float(_max_tick_y) - 1);
                    for (int i = 1; i < _max_tick_y-1; i++)
                    {
                        const QRect rect = QRect(_x+2, h - 1.5*space_for_ticks -2 - i*pixel_per_label_y, space_for_ticks, space_for_ticks);
                        float label = static_cast<float>(static_cast<int>((tick_label + (i-1) * tick_label) * 10.)) / 10.;
                        QString tmp_text;
                        if(_max_val < 20){
                            tmp_text = QString::number(std::ceil(label * 100.0) / 100.0);
                        } else {
                            tmp_text = QString::number((int)label);
                            if(tmp_text.size() >= 5){
                                tmp_text = tmp_text.mid(0,tmp_text.size()-3) + "k";
                            }
                        }
                        painter->drawText(rect, Qt::AlignLeft, tmp_text);
                        painter->drawLine(_x + space_for_ticks -4 +offset, h  - space_for_ticks - i*pixel_per_label_y
                                          , _x + space_for_ticks + offset, h - space_for_ticks - i*pixel_per_label_y);
                    }
                }
            }

            //bars
            double pixel_per_val;
            if(_norm){
                pixel_per_val = (h - space_for_ticks)/ (highest_val*1.0);
            } else {
                pixel_per_val = (h - space_for_ticks)/(_max_val*1.0);
            }
            double bar_width = 0.8;
            painter->setPen(Qt::NoPen);
            painter->setPen(QPen(QColor(128,128,128), 0));
            int num_labels = _freq_per_stage.size();
            if(num_labels > 20){
                num_labels = 20;
            }
            int pixel_per_label = floor((w- 2*space_for_ticks-offset)/num_labels);
            for (int i = 0; i < ind; i++)
            {
                int val = _freq_per_stage[labels[i]];
                qreal val_color = (val*1.0) / (_pat_id[labels[i]].size()*1.0);
                qreal prev_y = 0.0;
                for(int j=_pat_id[labels[i]].size()-1; j >= 0; j--){             
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
                    QRectF r (_x + space_for_ticks + i*pixel_per_label + offset + ((1-bar_width)/2)*pixel_per_label,
                              h - space_for_ticks - val*pixel_per_val + prev_y,
                              bar_width*pixel_per_label, val_color*pixel_per_val);
                    painter->drawRect(r);
                    prev_y += val_color*pixel_per_val;
                }
                _pos_rect[labels[i]] = {_x + space_for_ticks + i*pixel_per_label + offset + ((1-bar_width)/2)*pixel_per_label,
                                        h - space_for_ticks - val*pixel_per_val,bar_width*pixel_per_label,val*pixel_per_val};
            }
        } else if(_width_rect <= 100 && _width_rect > 30){
            double pixel_per_val;
            if(_norm){
                pixel_per_val = (h - space_for_ticks)/ (highest_val*1.0);
            } else {
                pixel_per_val = (h - space_for_ticks)/(_max_val*1.0);
            }
            painter->setPen(QPen(QColor(48,48,48), 0));
            int w_bar = _width_rect / ind;
            if(w_bar < 1){
                w_bar = 1;
            }
            for (int i = 0; i < ind; i++)
            {
                int val = _freq_per_stage[labels[i]];
                qreal val_color = (val*1.0) / (_pat_id[labels[i]].size()*1.0);
                qreal prev_y = 0.0;
                for(int j=_pat_id[labels[i]].size()-1; j >= 0; j--){
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
                    QRectF r (i*w_bar, h - space_for_ticks - val*pixel_per_val + prev_y,
                              w_bar, val_color*pixel_per_val);
                    painter->drawRect(r);
                    prev_y += val_color*pixel_per_val;
                }
                _pos_rect[labels[i]] = {static_cast<double>(i*w_bar), h - space_for_ticks - val*pixel_per_val,
                                        static_cast<double>(w_bar),val*pixel_per_val};
            }
       } else {
            painter->setPen(Qt::NoPen);
            double pixel_per_val;
            if(_norm){
                pixel_per_val = ((h - space_for_ticks - (not_zero-1)*2)/ (total*1.0));
            } else {
                pixel_per_val = ((h - space_for_ticks - (not_zero-1)*2)/ (_max_total_val*1.0));
            }
            qreal y = h - space_for_ticks;
            for (int i = 0; i < ind; i++)
            {
                if(_freq_per_stage[labels[i]] > 0){
                    int val = _freq_per_stage[labels[i]];
                    qreal val_color = (val*pixel_per_val) / (_pat_id[labels[i]].size()*1.0); //height of each event in current pat
                    qreal prev_y = 0.0;
                    for(int j=_pat_id[labels[i]].size()-1; j >= 0; j--){
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
                        QRectF r (0, y - val*pixel_per_val + prev_y,
                                  _width_rect,val_color);
                        painter->drawRect(r);
                        prev_y += val_color;
                    }                    
                    _pos_rect[labels[i]] = {0.0, y - val*pixel_per_val,
                                            _width_rect,val*pixel_per_val};
                    y -= val*pixel_per_val + 2;
                }
            }
        }

    } else {
        if(_width_rect > 100){
            //x-axis
            painter->drawLine(_x + space_for_ticks + offset, h - space_for_ticks, _x + w - space_for_ticks, h - space_for_ticks);
            //ticks x-axis
            int num_labels = _freq_per_stage.size();
            int pixel_per_label = floor((w- 2*space_for_ticks-offset)/num_labels);
            for (int i = 0; i < num_labels; i++)
            {
                const QRect rect = QRect(_x + space_for_ticks + i*pixel_per_label + offset, h  - space_for_ticks + 4 , pixel_per_label, space_for_ticks);
                painter->drawText(rect, Qt::AlignCenter, labels[i]);
                painter->drawLine(_x + space_for_ticks + i*pixel_per_label + pixel_per_label/2 + offset, h  - space_for_ticks,
                                   _x + space_for_ticks + i*pixel_per_label + pixel_per_label/2 + offset, h - space_for_ticks+4);
            }

            //y-axis
            painter->drawLine(_x + space_for_ticks +offset, h  - space_for_ticks , _x + space_for_ticks + offset, 0);
            //ticks y-axis
            if(_norm){
                for (int i = 0; i < 6; i++)
                {
                    const QRect rect_first = QRect(_x +2, h  - 1.5*space_for_ticks -2 - i*((h - space_for_ticks-10)/5.0),
                                                   space_for_ticks, space_for_ticks);
                    painter->drawText(rect_first, Qt::AlignLeft, QString::number(i*2) + "0");
                    painter->drawLine(_x + space_for_ticks -4 +offset, h - space_for_ticks - i*((h - space_for_ticks)/5.0),
                                      _x + space_for_ticks + offset, h - space_for_ticks - i*((h - space_for_ticks)/5.0));
                }
            } else {
                //first and last tick
                const QRect rect_first = QRect(_x +2, h  - 1.5*space_for_ticks -2 , space_for_ticks, space_for_ticks);
                painter->drawText(rect_first, Qt::AlignLeft, "0");
                painter->drawLine(_x + space_for_ticks -4 +offset, h - space_for_ticks, _x + space_for_ticks + offset, h - space_for_ticks);

                QString tmp_text = QString::number(_max_val);
                if(tmp_text.size() >= 5){
                    tmp_text = tmp_text.mid(0,tmp_text.size()-3) + "k";
                }
                const QRect rect_last = QRect(_x +2, 0, space_for_ticks, space_for_ticks);
                painter->drawText(rect_last, Qt::AlignLeft, tmp_text);
                painter->drawLine(_x + space_for_ticks-4 +offset, 0, _x + space_for_ticks +offset, 0 );

                //the other ticks
                if(_max_tick_y - 2 > 0)
                {
                    int pixel_per_label_y = floor((h - space_for_ticks)/(_max_tick_y -1)); //0 tick not included
                    float tick_label = float(_max_val)/(float(_max_tick_y) - 1);
                    for (int i = 1; i < _max_tick_y-1; i++)
                    {
                        float label = static_cast<float>(static_cast<int>((tick_label + (i-1) * tick_label) * 10.)) / 10.;
                        QString tmp_text;
                        if(_max_val < 20){
                            tmp_text = QString::number(std::ceil(label * 100.0) / 100.0);
                        } else {
                            tmp_text = QString::number((int)label);
                            if(tmp_text.size() >= 5){
                                tmp_text = tmp_text.mid(0,tmp_text.size()-3) + "k";
                            }
                        }
                        const QRect rect = QRect(_x+2, h - 1.5*space_for_ticks -2 - i*pixel_per_label_y, space_for_ticks, space_for_ticks);
                        painter->drawText(rect, Qt::AlignLeft, tmp_text);
                        painter->drawLine(_x + space_for_ticks -4 +offset, h  - space_for_ticks - i*pixel_per_label_y
                                          , _x + space_for_ticks + offset, h - space_for_ticks - i*pixel_per_label_y);

                    }
                }
            }

            //bars
            double pixel_per_val;
            if(_norm){
                pixel_per_val = (h - space_for_ticks)/ (highest_val*1.0);
            } else {
                pixel_per_val = (h - space_for_ticks)/(_max_val*1.0);
            }
            double bar_width = 0.8;
            painter->setPen(Qt::NoPen);
            painter->setPen(QPen(QColor(128,128,128), 0));
            for (int i = 0; i < num_labels; i++)
            {
                int val = _freq_per_stage[labels[i]];
                QColor col = _c[labels[i]];
                if(_high.size() == 1){
                    if(!_high.contains(labels[i])){
                       col = QColor(col.red(), col.green(), col.blue(), 50);
                   }
                }
                painter->setBrush(QBrush(col));
                painter->drawRect(_x + space_for_ticks + i*pixel_per_label + offset + ((1-bar_width)/2)*pixel_per_label,
                                  h - space_for_ticks - val*pixel_per_val,bar_width*pixel_per_label,val*pixel_per_val);
                _pos_rect[labels[i]] = {_x + space_for_ticks + i*pixel_per_label + offset + ((1-bar_width)/2)*pixel_per_label,
                                        h - space_for_ticks - val*pixel_per_val,bar_width*pixel_per_label,val*pixel_per_val};

            }
        } else if (_width_rect <= 100 && _width_rect > 20){
            double pixel_per_val;
            if(_norm){
                pixel_per_val = (h - space_for_ticks)/ (highest_val*1.0);
            } else {
                pixel_per_val = (h - space_for_ticks)/(_max_val*1.0);
            }
            painter->setPen(Qt::NoPen);
            int w_bar = _width_rect / ind;
            if(w_bar < 1){
                w_bar = 1;
            }
            for (int i = 0; i < ind; i++)
            {
                int val = _freq_per_stage[labels[i]];
                QColor col = _c[labels[i]];
                if(_high.size() ==1){
                    if(!_high.contains(labels[i])){
                       col = QColor(col.red(), col.green(), col.blue(), 50);
                   }
                }
                painter->setBrush(QBrush(col));
                painter->drawRect(i*w_bar, h - space_for_ticks - val*pixel_per_val,
                                  w_bar,val*pixel_per_val);
                _pos_rect[labels[i]] = {static_cast<double>(i*w_bar), h - space_for_ticks - val*pixel_per_val,
                                        static_cast<double>(w_bar),val*pixel_per_val};
            }
        } else {
            painter->setPen(Qt::NoPen);
            double pixel_per_val;
            if(_norm){
                pixel_per_val = (h - space_for_ticks)/ (total*1.0);
            } else {
                pixel_per_val = (h - space_for_ticks)/ (_max_total_val*1.0);
            }
            qreal y = h - space_for_ticks;
            for (int i = 0; i < ind; i++)
            {
                int val = _freq_per_stage[labels[i]];
                QColor col = _c[labels[i]];
                if(_high.size() ==1){
                    if(!_high.contains(labels[i])){
                       col = QColor(col.red(), col.green(), col.blue(), 50);
                   }
                }
                painter->setBrush(QBrush(col));
                QRectF r(0, y - val*pixel_per_val,
                         _width_rect,val*pixel_per_val);
                painter->drawRect(r);
                _pos_rect[labels[i]] = {0.0, y - val*pixel_per_val,
                                        _width_rect,val*pixel_per_val};
                y -= val*pixel_per_val;
            }
        }
    }
}

QRectF bar_chart_high::boundingRect() const
{
    return QRectF(0, 0, _width_rect, _height_rect);
}


void bar_chart_high::wheelEvent(QGraphicsSceneWheelEvent* event)
{
    if (event->delta() > 0)
    {
        this->setScale(this->scale() + 0.15);
    }
    else
    {
        this->setScale(this->scale() - 0.15);
    }

}

void bar_chart_high::mousePressEvent(QGraphicsSceneMouseEvent *mouseEvent){
     QString rect = "";
     for (auto i = _pos_rect.cbegin(), end = _pos_rect.cend(); i != end; ++i){
         if(mouseEvent->pos().rx() >= i.value()[0] &&
                 mouseEvent->pos().rx() <= i.value()[0] + i.value()[2] &&
                 mouseEvent->pos().ry() >= i.value()[1] &&
                 mouseEvent->pos().ry() <= i.value()[1] + i.value()[3]){
             rect = i.key();
             break;
         }
     }
     emit rectBarClicked(rect);

}
