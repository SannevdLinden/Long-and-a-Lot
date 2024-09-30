#include "mainwindow.h"
#include "bar_chart_high.h"
#include "staging.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QGraphicsRectItem>
#include <QPropertyAnimation>
#include <QVariantAnimation>
#include <QParallelAnimationGroup>
#include <QFile>
#include <QTableWidget>
#include <QColor>
#include "scatter_lines.h"
#include "tree.h"
#include "dots.h"
#include "grandparent.h"
#include "parent_low.h"
#include "child_low.h"
#include <set>
#include <QLabel>
#include <QVector>
#include <iostream>
#include <cmath>
#include <QPointF>
#include <QTransform>
#include <2d_colormap.h>
#include <QElapsedTimer>
#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>

// Library includes (temporary disable warnings)
#pragma GCC diagnostic ignored "-Wunknown-pragmas"
#pragma GCC diagnostic ignored "-Wunused-parameter"
#pragma GCC diagnostic ignored "-Wunused-but-set-parameter"
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wsign-compare"
//https://github.com/LTLA/umappp source of umap library
#include "umap/umappp/Umap.hpp"
#pragma GCC diagnostic warning "-Wunknown-pragmas"
#pragma GCC diagnostic warning "-Wunused-parameter"
#pragma GCC diagnostic warning "-Wunused-but-set-parameter"
#pragma GCC diagnostic warning "-Wunused-variable"
#pragma GCC diagnostic warning "-Wsign-compare"


//load data into a table like data frame
QTableWidget* MainWindow::load_data(QString fileData)
{
    //The file should contain three columns in this order: 'id' which is the seq id, 'event' which is the event cat and 'time'
    //make sure the last line of the csv file does not contain a white line, if so remove in file.
    QFile file(fileData);
    QString Alldata;
    QStringList rowAsString;
    QStringList row;
    QString cell;
    QStringList headers;
    QTableWidget* data_loaded = new QTableWidget();

    if (file.open(QFile::ReadOnly)){
           Alldata = file.readAll();
           rowAsString = Alldata.split("\n");
           file.close();
    }

    int rows_total = rowAsString.size()-1;
    data_loaded->setRowCount(rows_total);
    data_loaded->setColumnCount((rowAsString.at(0).split(",")).size());
    for (int i = 1; i < rowAsString.size(); i++){
           row = rowAsString.at(i).split(","); //get the column values
           if(row.size() > 1){
               for (int j = 0; j < row.size(); j++)
               {
                  if(j == 1)
                  {
                      cell = row[j].replace(QString("\r"), QString(""));
                      data_loaded->setItem(i-1,j,new QTableWidgetItem(cell));
                  }
                  else
                  {
                      data_loaded->setItem(i-1,j,new QTableWidgetItem(row[j]));
                  }
               }
               if(_total_count_event.count(row[1])){
                    _total_count_event[row[1]] ++;
               } else{
                    _total_count_event[row[1]] = 1;
               }
          }
    }
    headers = rowAsString.at(0).split(",");
    headers[1] = headers[1].replace(QString("\r"), QString(""));
    data_loaded -> setHorizontalHeaderLabels(headers);
    return data_loaded;
}

void MainWindow::get_seq_info(){
    QFile file(_seqFileName);
    QString Alldata;
    QStringList rowAsString;
    QStringList row;

    if (file.open(QFile::ReadOnly)){
           Alldata = file.readAll();
           rowAsString = Alldata.split("\n");
           file.close();
    }
    for (int i = 0; i < rowAsString.size(); i++){
           row = rowAsString.at(i).split(",");
           if(row.size() > 1){
              _seq_info[row[0].toInt()]= row[1];
           }
    }
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    srand (time(NULL));    
    _data = new QTableWidget();
    ui->dockWidget->setWindowFlag(Qt::CustomizeWindowHint);
    ui->dockWidget->setStyleSheet("QDockWidget {font-size:20px;}\n"
                "QDockWidget::title {background: none; border-style: none; padding: 5px;}\n"
                "QCheckBox::indicator {border : 2px solid rgb(128,128,128); border-radius :5px;}"
                 "QCheckBox::indicator:checked {background-color:rgb(128,128,128);}"
                                  "QWidget {background-color:rgb(64,64,64)}"
                                  "QPushButton {border: 2px solid rgb(128,128,128); border-radius :5px;}"
                                  "QPushButton::hover {background-color: rgb(128,128,128);}"
                                  "QListView::indicator {border : 2px solid rgb(128,128,128); border-radius :5px;}"
                                   "QListView::indicator:checked {background-color:rgb(128,128,128);}"
                                  "QListView::item:hover {background-color:rgb(48,48,48);}"
                                  "QProgressBar {border: 2px solid rgb(128,128,128);border-radius: 5px;}"
                                  "QProgressBar::chunk {background-color: rgb(128,128,128);}");
    //https://doc.qt.io/qt-6/stylesheet-examples.html#customizing-qdockwidget
    ui->label->setStyleSheet("font-size:15px;");
    ui->label_2->setStyleSheet("font-size:15px;");
    ui->label_3->setStyleSheet("font-size:15px;");
    ui->label_4->setStyleSheet("font-size:15px;");
    ui->label_10->setStyleSheet("font-size:15px;");
    ui->label_11->setStyleSheet("font-size:15px;");
    ui->progressBar->setVisible(false);
}

void MainWindow::init(){    
    _height =  ui->visual->height();
    _width = ui->visual->width();
    int height_stages = _height;
    int width_stages = _width;
    _scene = new QGraphicsScene();
    _scene->setSceneRect(0,0, width_stages, height_stages);
    ui->graphicsView_test->setScene(_scene);

    _scene_scatter = new QGraphicsScene();
    _scene_scatter->setSceneRect(0,0, 200, 200);
    ui->scatter->setScene(_scene_scatter);
    connect(ui->scatter,SIGNAL(boxSelection(bool)),this,SLOT(box_selection(bool)));
    ui->scatter->setAttribute(Qt::WA_AlwaysShowToolTips);
    ui->scatter->setStyleSheet("background-color:rgb(48,48,48); border: 0px;");
}

void MainWindow::draw_tree()
{
    qreal height_stages = _height - 20;
    int width_stages =_width;
    qreal x = 0;
    qreal y = 0;
    qreal width_rect = 0;
    int k =0;
    int offset_level =  0;
    QVector<QString> text;
    QVector<QString> already_done;
    QString mode = "minimized";
    QString parent = "done";
    int level_parent;
    double index_parent;
    //offset to align content rects
    for(int i=0;i< _levels.size();i++){
        for(int j=0; j< _levels[i].size(); j++){            
            if(_levels[i][j].split("-").size() == 1 &&
                    _levels[i][j] != ""){
               if(i > offset_level){
                   offset_level = i;
               }
            }
        }
    }
    //get max total val of a stage
    int max_val_stage = 0;
    for(int i = 0; i < _freq_per_stage.size(); i++){
        int total = 0;
        for (auto k = _freq_per_stage[i].cbegin(), end = _freq_per_stage[i].cend(); k != end; ++k){
            total += k.value();
        }
        if(total > max_val_stage){
            max_val_stage = total;
        }
    }
    //build top of tree
    for(int i=0;i< _levels.size();i++){        
        QVector<tree*> tmp_stage_rect;
        tmp_stage_rect.resize(_levels[i].size());
        QVector<QString> tmp_stage_status;
        tmp_stage_status.resize(_levels[i].size());
        QVector<QString> tmp_stage_high;
        tmp_stage_high.resize(_levels[i].size());
        QVector<QVector<QVector<child_low*>>> tmp_dwarf_willow;
        tmp_dwarf_willow.resize(_levels[i].size());
        QList<parent_low*> tmp_parent_low;
        tmp_parent_low.resize(_levels[i].size());
        QList<double> tmp_zoom;
        tmp_zoom.resize(_levels[i].size());
        QList<QPointF> tmp_pan;
        tmp_pan.resize(_levels[i].size());
        x = 0;
        already_done.clear();
        for(int j=0; j< _levels[i].size(); j++){            
            tmp_zoom[j] = 1.0;
            QPointF p(1.0,1.0);
            tmp_pan[j] = p;
            QMap<QString, double> freqs_level;
            text = _levels[i][j].split("-");
           // parent = "done";
            if(text.size() == 1 && _levels[i][j] != "done"){
               width_rect = width_stages / (qreal)_num_stages;
            } else if (text.size() > 1 && _levels[i][j] != "done"){
               width_rect = (text[1].toInt() - text[0].mid(1).toInt()+1) * width_stages / (qreal)_num_stages;
            } else if(_levels[i][j] == "done"){
               //find parent that is not done
                parent = "done";
                level_parent = i;
                index_parent = j;
                while (parent == "done"){
                   level_parent -= 1;
                   index_parent = ceil((index_parent +1)/2)-1;
                   parent = _levels[level_parent][index_parent];
               }
               //if parent not in already done list add and width following
               bool add = true;
               for(int q =0; q< already_done.size(); q++){
                   if(already_done[q] == parent){
                       add = false;
                   }
               }
               if (add){
                   width_rect = width_stages / (qreal)_num_stages;
                   already_done.append(parent);
                } else {
                   width_rect = 0;
               }
            }            
            y = i*_height_closed_rect;
            bool leave = false;
            if(_levels[i][j].split("-").size() <= 1){
                leave = true;
            }
            QVector<double> text;
            text.resize(2);
            text[0] = i;
            text[1] = j;
            if (_levels[i][j] != "done" && leave == false){
                int min;
                int max;
                if(_mode_staging_global == "patterns"){
                    //calc freqs for this level
                    min = _levels[i][j].split("-")[0].mid(1).toInt();
                    max = _levels[i][j].split("-")[1].toInt();
                    for(int s =min; s <= max; s++){
                        for (auto l = _pat_per_stage[s].cbegin(), end = _pat_per_stage[s].cend(); l != end; ++l){
                           freqs_level[l.key()] += l.value();
                        }
                    }
                } else {
                    //calc freqs for this level
                    min = _levels[i][j].split("-")[0].mid(1).toInt();
                    max = _levels[i][j].split("-")[1].toInt();
                    for(int s =min; s <= max; s++){
                        for (auto l = _freq_per_stage[s].cbegin(), end = _freq_per_stage[s].cend(); l != end; ++l){
                           freqs_level[l.key()] += l.value();
                        }
                    }
                }
                tree* boompje = new tree(width_rect, _height_closed_rect, "", false, 0, false, QColor(255,255,255),
                                         freqs_level, _c, _high_through_click, false,
                                         _mode_staging_global, _pat_id_to_pat, _most_freq_pat, {});
                boompje->setIndex(text);
                boompje->setPos(x,y);
                boompje->setCursor(Qt::ArrowCursor);
                boompje->setToolTip("Time stage: " + _start_end_time_stage[min].first
                                    + " / " + _start_end_time_stage[max].second);
                _scene->addItem(boompje);
                connect(boompje,SIGNAL(stageRectClicked(QVector<double>)),this,SLOT(stage_rect_clicked(QVector<double>)));
                tmp_stage_rect[j] = boompje;
                tmp_stage_status[j] = "false";
                tmp_stage_high[j] = "false";

            }
            if(_levels[i][j] != "done" && leave == true){
              k = _levels[i][j].mid(1).toInt();
              double value = 255*((1.0*k) / (1.0*_num_stages));
              tree* boompje = new tree(width_rect, height_stages-i*_height_closed_rect, _levels[i][j], false, _height_closed_rect*(offset_level-i),
                                       false, QColor(value,value,value, 175), freqs_level, _c, _high_through_click, false,
                                       _mode_staging_global, _pat_id_to_pat, _most_freq_pat, {});
              boompje->setIndex(text);
              boompje->setPos(x,y);
              //stage->setToolTip(QString::number(i));
              boompje->setCursor(Qt::ArrowCursor);
              boompje->setToolTip("Time stage: " + _start_end_time_stage[k].first
                                  + " / " + _start_end_time_stage[k].second);
              _scene->addItem(boompje);
              connect(boompje,SIGNAL(stageRectClicked(QVector<double>)),this,SLOT(stage_rect_clicked(QVector<double>)));
              tmp_stage_rect[j] = boompje;
              tmp_stage_status[j] = "true";
              tmp_stage_high[j] = "false";

              //grandparent set rect to put stuff
              qreal w = width_rect - 2*10 - 2;//-2 to compensate for border
              qreal h = height_stages-i*_height_closed_rect-23-2*10 - _height_closed_rect*(offset_level-i) -2;
              grandParent* westernBristleconePine = new grandParent(w, h, false);
              westernBristleconePine->setPos(11,_height_closed_rect*(offset_level-i)+23+1);
              westernBristleconePine->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
              westernBristleconePine->setParentItem(boompje);

              //zoomable space > parent
              QMap<QString, double> freq_stage;
              if(_mode_staging_global == "patterns"){
                 freq_stage = _pat_per_stage[k];
              } else {
                freq_stage = _freq_per_stage[k];
              }
              bar_chart_high* generalSherman = new bar_chart_high(w, h-2, _levels[i][j], freq_stage,
                                                _max_val, false, _c, _high_through_click, _mode_staging_global,
                                                                  _pat_id_to_pat, _most_freq_pat, {}, max_val_stage,
                                                                  _normalized_bar_charts);
              generalSherman->setPos(1,1);
              generalSherman->setParentItem(westernBristleconePine);
              connect(generalSherman,SIGNAL(rectBarClicked(QString)),this,SLOT(rect_bar_clicked(QString)));

            }
            x += width_rect;

        }
        _stage_rects.append(tmp_stage_rect);
        _stage_status.append(tmp_stage_status);
        _stage_high.append(tmp_stage_high);
        _dwarf_willows.append(tmp_dwarf_willow);
        _parent_lows.append(tmp_parent_low);
        _zoom.append(tmp_zoom);
        _zoom_mid_level.append(tmp_zoom);
        _max_sizes_seqs.append(tmp_zoom);
        _pan.append(tmp_pan);
        y += _height_closed_rect;
    }
    _max_val_leaves = _max_val;
}

void MainWindow::redraw_tree(QVector<double> changed, bool hover, bool animate){
    if(!animate){
        for(int i=0;i< _levels.size();i++){
            for(int j=0; j< _levels[i].size(); j++){
                if(_levels[i][j] != "done"){
                    tree* item = _stage_rects[i][j];
                    _scene->removeItem(item);
                    delete item;
                    _dwarf_willows[i][j].clear();
                }
            }
        }
    }
    if(!hover){
        //if parent of changed is open close it
        int level = changed[0];
        double index = changed[1];
        while (level > 0){
           level -= 1;
           index = ceil((index +1)/2)-1;
           if(_stage_status[level][index] == "true"){
              _stage_status[level][index] = "false";
           }
       }

        //if child of changed is open close it
        level = changed[0];
        index = changed[1];
        int index2 = changed[1];
        while(level < _levels.size()-1){
            level ++;
            index = index*2;
            index2 = index2*2 +1;
            for(int f= index; f <= index2; f++){
                if(_stage_status[level][f] == "true"){
                   _stage_status[level][f] = "false";
                }
            }
        }
    }

    int height_stages = _height -20;
    qreal x = 0;
    int y = 0;
    QList<QString> names;
    //get names highest parents that determine size
    QList<int> names_numbers;
    QVector<QString> text;
    int p = 0;
    for(int i=0;i< _levels.size();i++){
        for(int j=0; j< _levels[i].size(); j++){
            if(_stage_status[i][j] == "true"){ //rect opened
                text = _levels[i][j].mid(1).split("-");
                if(text.size() > 0){
                    bool add = true;
                    for(int k=0; k < names_numbers.size(); k++){
                        if(names_numbers[k] == text[0].toInt()){
                            add = false;
                        }
                    }
                    if(add){
                        names.append(_levels[i][j]);
                        names_numbers.append(text[0].toInt());
                        if (text.size() > 1){
                            p = text[0].toInt() +1;
                            while(p <= text[1].toInt()){
                               names_numbers.append(p);
                               p++;
                            }
                        }
                    }
                }
            }
        }
    }

    int offset_level = 0;
    //offset to align content rects
    for(int i=0;i< _levels.size();i++){
        for(int j=0; j< _levels[i].size(); j++){
            if(_stage_status[i][j] == "true"){
               if(i > offset_level){
                   offset_level = i;
               }
            }
        }
    }

    QString parent = "done";
    QVector<QString> already_done;
    QMap<QString, qreal> stage_width;
    QMap<QString, double> freq_stage;
    QList<int> y_pos = {0};
    int level_parent;
    double index_parent;
    qreal width_rect = 0;
    int height_rect = 0;
    int width_stages = _width;
    qreal w_open_stage = (width_stages - _width_closed_rect*(_num_stages-names_numbers.size()))/(qreal) names.size();
    QList<QList<int>> high_pat_event;
    bool high_pat = false;
    if(_mode_staging_global == "patterns"){
        high_pat_event = _high_patterns_events;
        if(_high_patterns_events.size() > 0){
            high_pat = true;
        }
    }
    //max val aanpassen for open rects that consists outof multiple stages
    _max_val = 0;
    //get the max total value of a stage
    int max_val_stage = 0;
    for(int i =0; i < names.size(); i++){
        int total_stage = 0;
        if(names[i].split("-").size() > 1){
            if(_mode_staging_global == "patterns"){
                freq_stage = _pat_per_stage[names[i].mid(1).split("-")[0].toInt()];
                for(int s = names[i].mid(1).split("-")[0].toInt()+1; s <= names[i].split("-")[1].toInt(); s++){
                    for (auto i = _pat_per_stage[s].cbegin(), end = _pat_per_stage[s].cend(); i != end; ++i) {
                       freq_stage[i.key()] += i.value();
                    }
                }
            } else {
                freq_stage = _freq_per_stage[names[i].mid(1).split("-")[0].toInt()];
                for(int s = names[i].mid(1).split("-")[0].toInt()+1; s <= names[i].split("-")[1].toInt(); s++){
                    for (auto i = _freq_per_stage[s].cbegin(), end = _freq_per_stage[s].cend(); i != end; ++i) {
                       freq_stage[i.key()] += i.value();
                    }
                }
            }
            for(auto i = freq_stage.cbegin(), end = freq_stage.cend(); i != end; ++i){
                if(!_filtered_out_cats.contains(i.key())){
                  total_stage += i.value();
                  if(_max_val < i.value()){
                    _max_val = i.value();
                  }
                }
            }
        } else {
            if(_mode_staging_global == "patterns"){
                freq_stage = _pat_per_stage[names[i].mid(1).toInt()];
            } else {
                freq_stage = _freq_per_stage[names[i].mid(1).toInt()];
            }

            for(auto i = freq_stage.cbegin(), end = freq_stage.cend(); i != end; ++i){
               if(!_filtered_out_cats.contains(i.key())){
                   if(_mode_staging_global == "patterns"){
                       if(_most_freq_pat.contains(i.key())){
                           total_stage += i.value();
                           if(_max_val < i.value()){

                             _max_val = i.value();
                           }
                       }
                   } else {
                       total_stage += i.value();
                       if(_max_val < i.value()){
                         _max_val = i.value();
                       }
                   }
               }
            }            
        }
        if(total_stage > max_val_stage){
            max_val_stage = total_stage;
        }
    }
    //width of the stages that are open on the highest level
    int k =0;    
    for(int i=0;i< _levels.size();i++){
        x = 0;
        QList<int> y_pos_tmp;
        already_done.clear();
        for(int j=0; j< _levels[i].size(); j++){            
            QMap<QString, double> freqs_level;
            parent = "done";
            if(_levels[i][j] != "done"){
                bool contains = false;
                for(int p=0; p< names.size(); p++){
                    if(names[p] == _levels[i][j]){
                        contains = true;
                    }
                }
                if(contains){
                    width_rect = w_open_stage;
                } else {
                    width_rect = calc_width_stages(names, names_numbers, _levels[i][j]);
                }
                QVector<double> text;
                text.resize(2);
                text[0] = i;
                text[1] = j;

                double index_parent = j;
                index_parent = ceil((index_parent +1)/2)-1;
                y = y_pos[index_parent];

                bool hover = false;
                if(_stage_high[i][j] == "true"){
                    hover = true;
                }

                if(_stage_status[i][j] == "false"){
                    int min_t, max_t;
                    if (_levels[i][j].split("-").size() ==1){
                        bool neither = true;
                        int min = _levels[i][j].mid(1).toInt();
                        min_t = min;
                        max_t = min;
                        for(int i=0; i< names_numbers.size(); i++){
                            if(names_numbers[i] >= min && names_numbers[i] <= min){
                               neither = false;
                               break;
                            }
                        }
                        if(neither){
                            height_rect = height_stages-i*_height_closed_rect;
                        } else {
                           height_rect = _height_closed_rect;
                        }
                        if(_mode_staging_global == "patterns"){
                            freqs_level = _pat_per_stage[_levels[i][j].mid(1).toInt()];
                        } else {
                            freqs_level = _freq_per_stage[_levels[i][j].mid(1).toInt()];
                        }
                    } else if(_levels[i][j].split("-").size() > 1){
                       height_rect = _height_closed_rect;
                       int min = _levels[i][j].split("-")[0].mid(1).toInt();
                       int max = _levels[i][j].split("-")[1].toInt();
                       min_t = min;
                       max_t = max;
                       if(_mode_staging_global == "patterns"){
                           for(int s =min; s <= max; s++){
                               for (auto l = _pat_per_stage[s].cbegin(), end = _pat_per_stage[s].cend(); l != end; ++l){
                                  freqs_level[l.key()] += l.value();
                               }
                           }
                       } else {
                           for(int s =min; s <= max; s++){
                               for (auto l = _freq_per_stage[s].cbegin(), end = _freq_per_stage[s].cend(); l != end; ++l){
                                  freqs_level[l.key()] += l.value();
                               }
                           }
                       }
                    }
                    for(int r = 0; r<_filtered_out_cats.size(); r++){
                       freqs_level.remove(_filtered_out_cats[r]);
                    }
                    for(int r=0; r<_seqs_filter.size(); r++){
                        if(!_seqs_filter[r]){
                            //get correct pieces of seqs and remove them from freq_stage
                            if(_levels[i][j].split("-").size() == 1){
                                int stage_id = _levels[i][j].mid(1).toInt();
                                for(int s=0; s< _seq_data_stages[stage_id][r].size(); s++){
                                    freqs_level[_seq_data_stages[stage_id][r][s].first] --;
                                }
                            } else {
                                int min = _levels[i][j].split("-")[0].mid(1).toInt();
                                int max = _levels[i][j].split("-")[1].toInt();
                                for(int m = min; m <= max; m++){
                                    for(int s=0; s< _seq_data_stages[m][r].size(); s++){
                                        freqs_level[_seq_data_stages[m][r][s].first] --;
                                    }
                                }
                            }
                        }
                    }
                    tree* boompje = new tree(width_rect, height_rect, "", hover, (_levels.size()-1-i)*_height_closed_rect,
                                             false, QColor(255,255,255), freqs_level, _c, _high_through_click, false,
                                             _mode_staging_global, _pat_id_to_pat, _most_freq_pat, _filtered_out_cats);
                    boompje->setIndex(text);
                    boompje->setPos(x,y);
                    boompje->setCursor(Qt::ArrowCursor);
                    boompje->setToolTip("Time stage: "+ _start_end_time_stage[min_t].first
                                        + " / " + _start_end_time_stage[max_t].second);
                    if(!animate){
                        _scene->addItem(boompje);
                    }
                    connect(boompje,SIGNAL(stageRectClicked(QVector<double>)),this,SLOT(stage_rect_clicked(QVector<double>)));
                    _stage_rects[i][j] = boompje;
                } else {
                    //height, open stage can be a leave, or there are still some levels beneath it that are not done.
                    int extra_space = 0;
                    if (_levels[i][j].split("-").size() ==1){
                        height_rect = height_stages-i*_height_closed_rect;
                    } else if(_levels[i][j].split("-").size() > 1){                        
                        int p = i;
                        int q1 = j;
                        int q2 = j;
                        while(p < _levels.size()-1){
                            p ++;
                            q1 = q1*2;
                            q2 = q2*2 +1;
                            for(int f= q1; f <= q2; f++){
                                if(_levels[p][f] != "done"){
                                    extra_space ++;
                                    break;
                                }
                            }
                        }
                        height_rect = height_stages-i*_height_closed_rect - extra_space*_height_closed_rect;
                    }
                    //values to display in bar chart
                    if(_levels[i][j].split("-").size() == 1){
                       k = _levels[i][j].mid(1).toInt();
                       if(_mode_staging_global == "patterns"){
                           freq_stage = _pat_per_stage[k];
                       } else {
                           freq_stage = _freq_per_stage[k];
                       }
                    } else {
                        if(_mode_staging_global == "patterns"){
                            freq_stage = _pat_per_stage[_levels[i][j].mid(1).split("-")[0].toInt()];
                            for(int s = _levels[i][j].mid(1).split("-")[0].toInt()+1; s <= _levels[i][j].split("-")[1].toInt(); s++){
                                for (auto i = _pat_per_stage[s].cbegin(), end = _pat_per_stage[s].cend(); i != end; ++i) {
                                   freq_stage[i.key()] += i.value();
                                }
                            }
                        } else {
                            freq_stage = _freq_per_stage[_levels[i][j].mid(1).split("-")[0].toInt()];
                            for(int s = _levels[i][j].mid(1).split("-")[0].toInt()+1; s <= _levels[i][j].split("-")[1].toInt(); s++){
                                for (auto i = _freq_per_stage[s].cbegin(), end = _freq_per_stage[s].cend(); i != end; ++i) {
                                   freq_stage[i.key()] += i.value();
                                }
                            }
                        }
                    }
                    bool cluster = false;
                    if(_cluster && _mode == "low"){
                        cluster = true;
                    }

                    QColor c_tree;
                    int min_t,max_t;
                    if(_color_scat_time){
                        double value = 0.0;
                        if(_levels[i][j].split("-").size() == 1){
                            value = (1.0*k) / (1.0*_num_stages);
                            min_t = k;
                            max_t = k;
                        } else if (_levels[i][j].split("-").size() > 1){
                            value = (1.0*_levels[i][j].mid(1).split("-")[0].toInt() + (0.5*(_levels[i][j].split("-")[1].toInt() -
                                     _levels[i][j].mid(1).split("-")[0].toInt()))) / (1.0*_num_stages);
                            min_t = _levels[i][j].mid(1).split("-")[0].toInt();
                            max_t = _levels[i][j].split("-")[1].toInt();
                        }
                        value = 255* value;
                        c_tree = QColor(value,value,value, 175);                        
                    } else {
                        if(_levels[i][j].split("-").size() == 1){
                            c_tree = _colors_dots[k];
                            min_t = k;
                            max_t = k;
                        } else if (_levels[i][j].split("-").size() > 1){
                            c_tree = _colors_dots[int((1.0*_levels[i][j].mid(1).split("-")[0].toInt() + (0.5*(_levels[i][j].split("-")[1].toInt() -
                                     _levels[i][j].mid(1).split("-")[0].toInt()))))];
                            min_t = _levels[i][j].mid(1).split("-")[0].toInt();
                            max_t = _levels[i][j].split("-")[1].toInt();
                        }
                    }
                    bool overflow_left = false;
                    if(_mode == "low" && _pan[i][j].rx() < 1){
                        overflow_left = true;
                    }                                      
                    tree* boompje = new tree(width_rect, height_rect, _levels[i][j], hover, _height_closed_rect*(offset_level-i),
                                             overflow_left, c_tree, freqs_level, _c, _high_through_click, false,
                                             _mode_staging_global, _pat_id_to_pat, _most_freq_pat, _filtered_out_cats);
                    boompje->setIndex(text);
                    boompje->setPos(x,y);
                    boompje->setCursor(Qt::ArrowCursor);
                    boompje->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
                    boompje->setToolTip("Time stage: "+ _start_end_time_stage[min_t].first
                                        + " / " + _start_end_time_stage[max_t].second);
                    if(!animate){
                        _scene->addItem(boompje);
                    }
                    connect(boompje,SIGNAL(stageRectClicked(QVector<double>)),this,SLOT(stage_rect_clicked(QVector<double>)));
                    _stage_rects[i][j] = boompje;

                    //grandparent set rect to put stuff
                    int w = width_rect - 2*10 -2;//-2 to compensate for border
                    int h = height_rect-2-23-2*10 - _height_closed_rect*(offset_level-i) - _height_closed_rect*(_levels.size()-offset_level - extra_space-1);
                    grandParent* westernBristleconePine = new grandParent(w, h, hover);
                    westernBristleconePine->setPos(11,_height_closed_rect*(offset_level-i)+23+1);
                    westernBristleconePine->setFlag(QGraphicsItem::ItemClipsChildrenToShape);
                    westernBristleconePine->setParentItem(boompje);
                    _width_open_parent_low = w;

                    if(_mode == "low" || _mode == "mid"){
                        //zoomable space > parent
                        parent_low* generalSherman = new parent_low(w, h, hover, QString::number(i) + "-" + QString::number(j), _mode);
                        generalSherman->setPos(1,1);
                        generalSherman->setParentItem(westernBristleconePine);                        
                        _parent_lows[i][j] = generalSherman;
                        connect(generalSherman,SIGNAL(zoom(QPair<double, QString>)),this,SLOT(linked_zoom(QPair<double, QString>)));
                        connect(generalSherman,SIGNAL(zoom_mid_level(QPair<double, QString>)),this,SLOT(factor_mid_level(QPair<double, QString>)));
                        connect(generalSherman,SIGNAL(pan(QPair<QPointF, QString>)),this,SLOT(linked_pan(QPair<QPointF, QString>)));

                        //grandchild > all blocks
                        QList<QList<QPair<QString, int>>> seq_data_stage;
                        int lowest_index;
                        if(_levels[i][j].split("-").size() == 1){
                            int number = _levels[i][j].mid(1).toInt();
                            seq_data_stage = _seq_data_stages[number];
                            lowest_index = _lowest_ind_stage[number];
                        } else if (_levels[i][j].split("-").size() > 1){
                           int min = _levels[i][j].mid(1).split("-")[0].toInt();
                           int max = _levels[i][j].split("-")[1].toInt();
                           lowest_index = _lowest_ind_stage[min];
                           seq_data_stage = _seq_data_stages[min];
                           for(int w=min+1; w<= max;w++){
                               for(int s=0; s< seq_data_stage.size();s++){
                                   seq_data_stage[s].append(_seq_data_stages[w][s]);
                               }
                           }
                        }

                        int row_margin = 1;
                        int x_blocks = 0;
                        int y_blocks = 0;
                        QMap<QString, int> count_mid_comp;
                        QMap<QString, int> count_mid_comp_pat;
                        //compute the count per window piece in the mid level
                        //compute compression factor
                        int comp_factor = ceil((_max_sizes_seqs[i][j]*_block_size / w)*_zoom_mid_level[i][j]);

                        if (! _cluster){
                            QVector<QVector<child_low*>> tmp_dwarf;
                            tmp_dwarf.resize(seq_data_stage.size());
                            for(int s=0; s< seq_data_stage.size();s++){
                                if(_seqs_filter[s]){
                                    if(_mode == "low"){
                                        x_blocks = (seq_data_stage[s][0].second - lowest_index) * (_block_size+_block_margin);
                                    } else {
                                        x_blocks = 0;
                                    }
                                    bool pat_detect = false;
                                    int count_pat = 0;
                                    if(seq_data_stage[s].size() + (seq_data_stage[s][0].second - lowest_index) > _max_sizes_seqs[i][j]){
                                        _max_sizes_seqs[i][j] = seq_data_stage[s].size() + (seq_data_stage[s][0].second - lowest_index);
                                        comp_factor = ceil((_max_sizes_seqs[i][j]*_block_size / w)*_zoom_mid_level[i][j]);
                                    }


                                    for(int u=0; u< seq_data_stage[s].size();u++){
                                        int max_val_comp = 0;
                                        QString cat_comp;
                                        int max_val_comp_pat = 0;
                                        QString cat_comp_pat;
                                        if(_mode == "mid"){
                                            count_mid_comp[seq_data_stage[s][u].first] += 1;
                                            if ((u+1) % comp_factor == 0 || u == seq_data_stage[s].size()-1){
                                                //count_mid_comp is an ordered list, so if all the categories have the same count, then picks the first one alphabetically
                                                for (auto [key, value] : count_mid_comp.asKeyValueRange()) {
                                                    if(value > max_val_comp){
                                                        max_val_comp = value;
                                                        cat_comp = key;
                                                    }
                                                }
                                            }
                                        }                                        
                                        QList<QString> tmp_high;
                                        //check for pattern
                                        if(!_pat_created){
                                             tmp_high = _high_through_click;
//
                                        } else {
                                            if(_mode == "low"){
                                                if(seq_data_stage[s][u].first == _high_through_click[0] && !pat_detect){
                                                    pat_detect = true;
                                                    for(int q=1; q < _high_through_click.size(); q++){
                                                        if(u+q < seq_data_stage[s].size()){
                                                            if(seq_data_stage[s][u+q].first != _high_through_click[q]){
                                                                pat_detect = false;
                                                                break;
                                                            }
                                                        } else {
                                                            pat_detect = false;
                                                            break;
                                                        }
                                                    }
                                                }
                                            } else {                     
                                                if ((u+1) % comp_factor == 0 || u == seq_data_stage[s].size()-1){
                                                    if(cat_comp == _high_through_click[0] && !pat_detect){
                                                        pat_detect = true;
                                                        count_mid_comp_pat.clear();
                                                        for(int q=1; q <= ((_high_through_click.size()-1)*comp_factor); q++){
                                                            if(u+q < seq_data_stage[s].size()){
                                                                //get average category
                                                                count_mid_comp_pat[seq_data_stage[s][u+q].first] += 1;
                                                                if ((q) % comp_factor == 0){
                                                                    for (auto [key, value] : count_mid_comp_pat.asKeyValueRange()) {
                                                                        if(value > max_val_comp_pat){
                                                                            max_val_comp_pat = value;
                                                                            cat_comp_pat = key;
                                                                        }
                                                                    }
                                                                    //check if that cat is the next of the pattern
                                                                    if(cat_comp_pat != _high_through_click[q/comp_factor]){
                                                                        pat_detect = false;
                                                                        break;
                                                                    }
                                                                    count_mid_comp_pat.clear();
                                                                 }
                                                            } else {
                                                                pat_detect = false;
                                                                break;
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            if(_mode == "low"){
                                                if(pat_detect && count_pat < _high_through_click.size()){
                                                    tmp_high = _high_through_click;
                                                    count_pat ++;
                                                    if(count_pat >= _high_through_click.size()){
                                                        count_pat = 0;
                                                        pat_detect = false;
                                                    }
                                                } else {
                                                    tmp_high = {"no-cat"};
                                                }
                                            } else {
                                                if(pat_detect && count_pat < _high_through_click.size()*comp_factor){
                                                    tmp_high = _high_through_click;
                                                    count_pat ++;
                                                    if(count_pat >= _high_through_click.size()*comp_factor){
                                                        count_pat = 0;
                                                        pat_detect = false;
                                                    }
                                                } else {
                                                    tmp_high = {"no-cat"};
                                                }
                                            }
                                        }
                                        if(!_filtered_out_cats.contains(seq_data_stage[s][u].first) || _mode == "mid"){
                                            bool non_high_color_pat = false;
                                            int index_pat = -1;
                                            if(high_pat && !(_mode == "mid" && _mode_staging_global != "entropy")){
                                               non_high_color_pat = true;
                                               for(int z =0; z < high_pat_event[s].size(); z++){
                                                if(high_pat_event[s][z] == seq_data_stage[s][u].second){
                                                    non_high_color_pat = false;
                                                    index_pat = z;
                                                    break;
                                                }
                                                if(index_pat >= 0){
                                                   high_pat_event[s].removeAt(index_pat);
                                                   index_pat = -1;
                                                }
                                              }

                                            }                                            
                                            if(_mode == "low"){                                                
                                                child_low* dwarfWillow = new child_low(_block_size,_block_size,
                                                                                       seq_data_stage[s][u].first, s, _c, tmp_high, true,
                                                                                       QString::number(i) + "/" + QString::number(j), non_high_color_pat, high_pat);
                                                dwarfWillow->setPos(x_blocks, y_blocks);                                            
                                                dwarfWillow->setParentItem(generalSherman);
                                                QString time = _data->item(seq_data_stage[s][u].second+s*_len_seq,2)->text();
                                                time = time.mid(0,10) + " " + time.mid(11,8);
                                                dwarfWillow->setToolTip(seq_data_stage[s][u].first + " " + time
                                                                       + " " + _seq_info[s] );
                                                connect(dwarfWillow,SIGNAL(rectEventClicked(QString)),this,SLOT(rect_event_clicked(QString)));
                                                tmp_dwarf[s].append(dwarfWillow);
                                            } else {
                                                if ((u+1) % comp_factor == 0 || u == seq_data_stage[s].size()-1){

                                                    if(!_filtered_out_cats.contains(cat_comp)){
                                                        child_low* dwarfWillow = new child_low(_block_size,_block_size,
                                                                                               cat_comp, s, _c, tmp_high, true,
                                                                                               QString::number(i) + "/" + QString::number(j), non_high_color_pat, high_pat);
                                                        dwarfWillow->setPos(x_blocks, y_blocks);
                                                        dwarfWillow->setParentItem(generalSherman);                                                  
                                                        QString time_start = _data->item(seq_data_stage[s][u-comp_factor+1].second+s*_len_seq,2)->text();
                                                        time_start = time_start.mid(0,10) + " " + time_start.mid(11,8);
                                                        QString time_end = _data->item(seq_data_stage[s][u].second+s*_len_seq,2)->text();
                                                        time_end = time_end.mid(0,10) + " " + time_end.mid(11,8);
                                                        dwarfWillow->setToolTip(cat_comp + " " + time_start +  "-" + time_end +
                                                                               + " " + _seq_info[s] );
                                                        connect(dwarfWillow,SIGNAL(rectEventClicked(QString)),this,SLOT(rect_event_clicked(QString)));
                                                        tmp_dwarf[s].append(dwarfWillow);

                                                        count_mid_comp.clear();
                                                        x_blocks += _block_margin + _block_size;
                                                    } else {
                                                        count_mid_comp.clear();
                                                    }
                                                }
                                            }
                                        } else if (_mode == "mid"){ //due to filtering might skip the clear
                                            if ((u+1) % comp_factor == 0 || u == seq_data_stage[s].size()-1){
                                                count_mid_comp.clear();
                                            }
                                        }
                                        if(_mode == "low"){
                                            x_blocks += _block_margin + _block_size;
                                        }
                                    }
                                    y_blocks += _block_size + row_margin;
                                }
                            }
                            if(y_blocks < 1000){
                                y_blocks = 1000;
                            }
                            if(x_blocks < 2000){
                                x_blocks = 2000;
                            }
                            generalSherman -> set_bounding(x_blocks, y_blocks);
                            _dwarf_willows[i][j] = tmp_dwarf;
                        } else {
                            QVector<QVector<child_low*>> tmp_dwarf;
                            tmp_dwarf.resize(seq_data_stage.size());

                            int min_c = _levels[i][j].mid(1).split("-")[0].toInt();
                            int max_c = 0;
                            if(_levels[i][j].split("-").size() > 1){
                                max_c = _levels[i][j].split("-")[1].toInt();
                            } else {
                                max_c = min_c;
                            }
                            int prev_block_x = 0;
                            bool high_hover = false;
                            for(int c =min_c; c < max_c+1; c++){
                                seq_data_stage = _seq_data_stages[c];
                                y_blocks = 0;
                                QList<int> key;
                                std::map<int, QList<int>> stage_clusters = _cluster_info[c];
                                if(_cluster_on >= 0){
                                    stage_clusters = _cluster_info[_cluster_on];                                    
                                }
                                for(std::map<int,QList<int>>::iterator it = stage_clusters.begin(); it != stage_clusters.end(); ++it) {
                                  key.append(it->first);
                                }

                                for(int s=0; s< key.size();s++){                                    
                                    for(int u =0; u<stage_clusters[key[s]].size(); u++){                                        
                                        int ind_tmp = stage_clusters[key[s]][u];
                                        if(_mode == "low"){
                                            x_blocks = (seq_data_stage[ind_tmp][0].second - lowest_index) * (_block_size+_block_margin) + prev_block_x;
                                        } else {
                                            x_blocks = 0;
                                        }
                                        bool pat_detect = false;
                                        int count_pat = 0;
                                        if(_seqs_filter[ind_tmp]){
                                            if(seq_data_stage[ind_tmp].size() + (seq_data_stage[ind_tmp][0].second - lowest_index) > _max_sizes_seqs[i][j]){
                                                _max_sizes_seqs[i][j] = seq_data_stage[ind_tmp].size() + (seq_data_stage[ind_tmp][0].second - lowest_index);
                                                comp_factor = ceil((_max_sizes_seqs[i][j]*_block_size / w)*_zoom_mid_level[i][j]);
                                            }
                                            for(int w=0; w< seq_data_stage[ind_tmp].size();w++){
                                                int max_val_comp = 0;
                                                QString cat_comp;
                                                int max_val_comp_pat = 0;
                                                QString cat_comp_pat;
                                                if(_mode == "mid"){
                                                    count_mid_comp[seq_data_stage[ind_tmp][w].first] += 1;
                                                    if ((w+1) % comp_factor == 0 || w == seq_data_stage[ind_tmp].size()-1){
                                                        for (auto [key, value] : count_mid_comp.asKeyValueRange()) {
                                                            if(value > max_val_comp){
                                                                max_val_comp = value;
                                                                cat_comp = key;
                                                            }
                                                        }
                                                    }
                                                }
                                                QList<QString> tmp_high;
                                                //check for pattern
                                                if(!_pat_created){
                                                    tmp_high = _high_through_click;
                                                } else {
                                                    if(_mode == "low"){
                                                        if(seq_data_stage[ind_tmp][w].first == _high_through_click[0] && !pat_detect){
                                                            pat_detect = true;
                                                            for(int q=1; q < _high_through_click.size(); q++){
                                                                if(w+q < seq_data_stage[ind_tmp].size()){
                                                                    if(seq_data_stage[ind_tmp][w+q].first != _high_through_click[q]){
                                                                        pat_detect = false;
                                                                        break;
                                                                    }
                                                                } else {
                                                                    pat_detect = false;
                                                                    break;
                                                                }
                                                            }
                                                        }
                                                        if(pat_detect && count_pat < _high_through_click.size()){
                                                            tmp_high = _high_through_click;
                                                            count_pat ++;
                                                            if(count_pat >= _high_through_click.size()){
                                                                count_pat = 0;
                                                                pat_detect = false;
                                                            }
                                                        } else {
                                                            tmp_high = {"no-cat"};
                                                        }
                                                    } else {
                                                        if ((w+1) % comp_factor == 0 || w == seq_data_stage[ind_tmp].size()-1){
                                                            if(cat_comp == _high_through_click[0] && !pat_detect){
                                                                pat_detect = true;
                                                                count_mid_comp_pat.clear();
                                                                for(int q=1; q <= ((_high_through_click.size()-1)*comp_factor); q++){
                                                                    if(w+q < seq_data_stage[ind_tmp].size()){
                                                                        //get average category
                                                                        count_mid_comp_pat[seq_data_stage[ind_tmp][w+q].first] += 1;
                                                                        if ((q) % comp_factor == 0){
                                                                            for (auto [key, value] : count_mid_comp_pat.asKeyValueRange()) {
                                                                                if(value > max_val_comp_pat){
                                                                                    max_val_comp_pat = value;
                                                                                    cat_comp_pat = key;
                                                                                }
                                                                            }
                                                                            //check if that cat is the next of the pattern
                                                                            if(cat_comp_pat != _high_through_click[q/comp_factor]){
                                                                                pat_detect = false;
                                                                                break;
                                                                            }
                                                                            count_mid_comp_pat.clear();
                                                                         }
                                                                    } else {
                                                                        pat_detect = false;
                                                                        break;
                                                                    }
                                                                }
                                                            }
                                                        }

                                                        if(pat_detect && count_pat < _high_through_click.size()*comp_factor){
                                                            tmp_high = _high_through_click;
                                                            count_pat ++;
                                                            if(count_pat >= _high_through_click.size()*comp_factor){
                                                                count_pat = 0;
                                                                pat_detect = false;
                                                            }
                                                        } else {
                                                            tmp_high = {"no-cat"};
                                                        }
                                                    }

                                                }
                                                if(_hover_cluster_ind == -1){
                                                    high_hover = true; //no highlight
                                                } else{ //highlight
                                                    if(_mode_staging_global == "patterns"){
                                                        high_pat = false;
                                                        if(tmp_high.size() > 0){
                                                            if(!_cats.contains(tmp_high[0])){
                                                                tmp_high.clear();
                                                            }
                                                        }
                                                    }
                                                    if(ind_tmp == _hover_cluster_ind){
                                                        high_hover = true;
                                                    } else {
                                                        high_hover = false;
                                                    }
                                                }
                                                if(!_filtered_out_cats.contains(seq_data_stage[ind_tmp][w].first) || _mode == "mid"){
                                                    bool non_high_color_pat = false;
                                                    if(_mode == "low"){
                                                        if(high_pat && _mode_staging_global == "entropy"){
                                                           int index_pat = -1;
                                                           non_high_color_pat = true;
                                                            for(int z =0; z < high_pat_event[ind_tmp].size(); z++){
                                                                if(high_pat_event[ind_tmp][z] == seq_data_stage[ind_tmp][w].second){
                                                                    non_high_color_pat = false;
                                                                    index_pat = z;
                                                                    break;
                                                                }
                                                            }
                                                            if(index_pat >= 0){
                                                               high_pat_event[ind_tmp].removeAt(index_pat);
                                                               index_pat = -1;
                                                            }
                                                        }
                                                        child_low* dwarfWillow = new child_low(_block_size,_block_size,
                                                                                               seq_data_stage[ind_tmp][w].first, ind_tmp, _c, tmp_high, high_hover,
                                                                                               QString::number(i) + "/" + QString::number(j), non_high_color_pat, high_pat);
                                                        dwarfWillow->setPos(x_blocks, y_blocks);
                                                        dwarfWillow->setParentItem(generalSherman);
                                                        QString time = _data->item(seq_data_stage[ind_tmp][w].second+ind_tmp*_len_seq,2)->text();
                                                        time = time.mid(0,10) + " " + time.mid(11,8);
                                                        dwarfWillow->setToolTip(seq_data_stage[ind_tmp][w].first + " " + time
                                                                                + " " + _seq_info[ind_tmp]);
                                                        connect(dwarfWillow,SIGNAL(rectEventClicked(QString)),this,SLOT(rect_event_clicked(QString)));
                                                        connect(dwarfWillow,SIGNAL(seqHigh(int)),this,SLOT(seq_high_cluster(int)));
                                                        connect(dwarfWillow,SIGNAL(seqLow(int)),this,SLOT(seq_low_cluster(int)));

                                                        tmp_dwarf[ind_tmp].append(dwarfWillow);
                                                    } else {                                                        
                                                        if ((w+1) % comp_factor == 0 || w == seq_data_stage[ind_tmp].size()-1){                                                         
                                                            if(!_filtered_out_cats.contains(cat_comp)){
                                                                child_low* dwarfWillow = new child_low(_block_size,_block_size,
                                                                                                       cat_comp, ind_tmp, _c, tmp_high, high_hover,
                                                                                                       QString::number(i) + "/" + QString::number(j), non_high_color_pat, high_pat);
                                                                dwarfWillow->setPos(x_blocks, y_blocks);
                                                                dwarfWillow->setParentItem(generalSherman);
                                                                QString time_start = _data->item(seq_data_stage[ind_tmp][w-comp_factor+1].second+s*_len_seq,2)->text();
                                                                time_start = time_start.mid(0,10) + " " + time_start.mid(11,8);
                                                                QString time_end = _data->item(seq_data_stage[ind_tmp][w].second+s*_len_seq,2)->text();
                                                                time_end = time_end.mid(0,10) + " " + time_end.mid(11,8);
                                                                dwarfWillow->setToolTip(cat_comp + " " + time_start + "-" + time_end +
                                                                                        + " " + _seq_info[ind_tmp]);
                                                                connect(dwarfWillow,SIGNAL(rectEventClicked(QString)),this,SLOT(rect_event_clicked(QString)));
                                                                connect(dwarfWillow,SIGNAL(seqHigh(int)),this,SLOT(seq_high_cluster(int)));
                                                                connect(dwarfWillow,SIGNAL(seqLow(int)),this,SLOT(seq_low_cluster(int)));

                                                                tmp_dwarf[ind_tmp].append(dwarfWillow);

                                                                count_mid_comp.clear();
                                                                x_blocks += _block_margin + _block_size;
                                                            } else {
                                                                count_mid_comp.clear();
                                                            }
                                                        }
                                                    }
                                                }  else if (_mode == "mid"){ //due to filtering might skip the clear
                                                    if ((w+1) % comp_factor == 0 || w == seq_data_stage[ind_tmp].size()-1){
                                                        count_mid_comp.clear();
                                                    }
                                                }
                                                if(_mode == "low"){
                                                    x_blocks += _block_margin + _block_size;
                                                }

                                            }
                                            y_blocks += (_block_size + row_margin);
                                          }
                                    }
                                    y_blocks += 10;
                                }
                                prev_block_x += 3*_block_size;
                            }
                            _dwarf_willows[i][j] = tmp_dwarf;
                        }
                        if(_mode == "low"){
                         generalSherman -> setScaleFunc(_zoom[i][j]);
                        }
                        generalSherman -> setPosFunc(_pan[i][j]);
                        if(w < _max_sizes_seqs[i][j]*(_block_size+_block_margin)*_zoom[i][j]){
                            boompje -> overflow_right(true);
                        }
                    } else if(_mode == "high"){
                        //zoomable space > parent
                        for(int r = 0; r<_filtered_out_cats.size(); r++){
                           freq_stage.remove(_filtered_out_cats[r]);
                        }
                        for(int r=0; r<_seqs_filter.size(); r++){
                            if(!_seqs_filter[r]){
                                //get correct pieces of seqs and remove them from freq_stage
                                if(_mode_staging_global == "patterns"){
                                    if(_levels[i][j].split("-").size() == 1){
                                        int stage_id = _levels[i][j].mid(1).toInt();
                                        for (auto z = _pat_per_stage_seq[stage_id][r].cbegin(), end = _pat_per_stage_seq[stage_id][r].cend(); z != end; ++z){
                                           if(z.value() > 0){
                                                freq_stage[z.key()] -= z.value();
                                           }
                                        }
                                    } else {
                                        int min = _levels[i][j].split("-")[0].mid(1).toInt();
                                        int max = _levels[i][j].split("-")[1].toInt();
                                        for(int m = min; m <= max; m++){
                                            for (auto z = _pat_per_stage_seq[m][r].cbegin(), end = _pat_per_stage_seq[m][r].cend(); z != end; ++z){
                                               if(z.value() > 0){
                                                    freq_stage[z.key()] -= z.value();
                                               }
                                            }
                                        }
                                    }

                                } else {
                                    if(_levels[i][j].split("-").size() == 1){
                                        int stage_id = _levels[i][j].mid(1).toInt();                                        
                                        for(int s=0; s< _seq_data_stages[stage_id][r].size(); s++){
                                            freq_stage[_seq_data_stages[stage_id][r][s].first] --;
                                        }
                                    } else {
                                        int min = _levels[i][j].split("-")[0].mid(1).toInt();
                                        int max = _levels[i][j].split("-")[1].toInt();
                                        for(int m = min; m <= max; m++){
                                            for(int s=0; s< _seq_data_stages[m][r].size(); s++){
                                                freq_stage[_seq_data_stages[m][r][s].first] --;
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        bar_chart_high* generalSherman = new bar_chart_high(w, h-2, _levels[i][j], freq_stage, _max_val,
                                                              hover, _c, _high_through_click, _mode_staging_global,
                                                              _pat_id_to_pat, _most_freq_pat, _filtered_out_cats, max_val_stage,
                                                               _normalized_bar_charts);
                        generalSherman->setPos(0,0);
                        generalSherman->setParentItem(westernBristleconePine);
                        connect(generalSherman,SIGNAL(rectBarClicked(QString)),this,SLOT(rect_bar_clicked(QString)));
                    }

                }
                if(_levels[i][j].split("-").size() == 1){
                    stage_width.insert(_levels[i][j], width_rect);
                }
                x += width_rect;
                y_pos_tmp.append(y+height_rect);
            } else {
                //find parent that is not done
                level_parent = i;
                index_parent = j;
                while (parent == "done"){
                    level_parent -= 1;
                    index_parent = ceil((index_parent +1)/2)-1;
                    parent = _levels[level_parent][index_parent];
                }
                //if parent not in already done list add and width following
                bool add = true;
                for(int q =0; q< already_done.size(); q++){
                    if(already_done[q] == parent){
                        add = false;
                    }
                }
                if (add){
                    x += stage_width[parent];
                    already_done.append(parent);
                 }
                y_pos_tmp.append(0);
            }
        }
        y_pos = y_pos_tmp;
    }
}

void MainWindow::draw_scatter()
{
    double width_height = 200;
    //axes and background
    scatter_lines* scat = new scatter_lines(width_height, width_height);
    scat->setPos(0,0);
    _scene_scatter->addItem(scat);
    _scatter_points.resize(_num_stages);

    QList<QList<double>> dist_metr;
    dist_metr.resize(_num_stages);
    std::vector<std::string> cats_vec;
    std::vector<bool> cats_filter_vec;
    for(int i =0; i< _cats.size();i++){
       cats_vec.push_back(_cats[i].toStdString());
    }
    for(int i =0; i< _cats_filter.size();i++){
       cats_filter_vec.push_back(_cats_filter[i]);
    }

    for(int j=0;j<_num_stages; j++){
        for(int k=0;k<_num_stages; k++){
            if(k <= j){
                if (k == j){
                    dist_metr[j].append(0);
                } else {
                    QMap<QString, double> total_counts;
                    bool all_zero_a = true;
                    bool all_zero_b = true;
                    // needed for info gain pattern distance
                    if(_mode_staging_global == "patterns"){
                        total_counts = _pat_per_stage[j];
                        for (auto i = _pat_per_stage[k].cbegin(), end = _pat_per_stage[k].cend(); i != end; ++i){
                            if(total_counts[i.key()] > 0.0){
                                all_zero_a = false;
                            }
                            if(_pat_per_stage[k][i.key()] > 0.0){
                                all_zero_b = false;
                            }
                            total_counts[i.key()] += i.value();

                        }
                    } else {
                        total_counts = _freq_per_stage[j];
                        for (auto i = _freq_per_stage[k].cbegin(), end = _freq_per_stage[k].cend(); i != end; ++i){
                            total_counts[i.key()] += i.value();
                        }
                    }
                    double dist;
                    if(_mode_staging_global == "patterns"){
                        //dist entropy and info gain
                        if (all_zero_a && all_zero_a){
                            dist = 0;
                        } else if(all_zero_a || all_zero_b){
                            dist = 1;
                        } else {                            
                            std::unordered_map<std::string, double> total_counts_map;
                            std::unordered_map<std::string, double> counts_left_map;
                            std::unordered_map<std::string, double> counts_right_map;
                            for (auto k = total_counts.cbegin(), end = total_counts.cend(); k != end; ++k){
                                total_counts_map.insert({k.key().toStdString(), k.value()});
                            }
                            for (auto k = _pat_per_stage[j].cbegin(), end = _pat_per_stage[j].cend(); k != end; ++k){
                                counts_left_map.insert({k.key().toStdString(), k.value()});
                            }
                            for (auto l = _pat_per_stage[k].cbegin(), end = _pat_per_stage[k].cend(); l != end; ++l){
                                counts_right_map.insert({l.key().toStdString(), l.value()});
                            }
                            dist = info_gain(counts_left_map, counts_right_map, total_counts_map, cats_vec, cats_filter_vec);
                        }
                    } else {
                        //dist entropy and info gain
                        std::unordered_map<std::string, double> total_counts_map;
                        std::unordered_map<std::string, double> counts_left_map;
                        std::unordered_map<std::string, double> counts_right_map;
                        for (auto k = total_counts.cbegin(), end = total_counts.cend(); k != end; ++k){
                            total_counts_map.insert({k.key().toStdString(), k.value()});
                        }
                        for (auto k = _freq_per_stage[j].cbegin(), end = _freq_per_stage[j].cend(); k != end; ++k){
                            counts_left_map.insert({k.key().toStdString(), k.value()});
                        }
                        for (auto l = _freq_per_stage[k].cbegin(), end = _freq_per_stage[k].cend(); l != end; ++l){
                            counts_right_map.insert({l.key().toStdString(), l.value()});
                        }
                        dist = info_gain(counts_left_map, counts_right_map, total_counts_map, cats_vec, cats_filter_vec);
                    }
                    if(dist == 0){
                        dist = 0.00000001;
                    }
                    dist_metr[j].append(dist);
                    dist_metr[k].append(dist);
                }
            }
        }
    }

    //https://github.com/LTLA/umappp source of umap library
    std::vector<std::vector<std::pair<int, double> > > distance_umap;
    for (int j =0; j< dist_metr.size(); j++) {
        std::vector<std::pair<int, double> > neighbors;
        //neighbors based on threshold, very sensetive to number of neighbors
//        for(int q=0; q<dist_metr[j].size(); q++){
//            if(dist_metr[j][q] > 0 && dist_metr[j][q] <= 0.05){
//                std::pair<int, double> indexDistance(q, dist_metr[j][q]);
//                neighbors.push_back(indexDistance);
//            }
//        }
        std::vector<std::pair<double, int> > sorted;
        for(int q=0; q<dist_metr[j].size(); q++){
           sorted.push_back(std::make_pair(dist_metr[j][q], q));
        }
        std::sort(sorted.begin(), sorted.end());
        for(int q=1; q<= 6; q++){ //q=0 is the 0 diagonal
            if(sorted.size()-1 >= q){
                std::pair<int, double> indexDistance(sorted[q].second, sorted[q].first);
                neighbors.push_back(indexDistance);
            }
        }
        distance_umap.push_back(neighbors);
    }
    std::vector<double> embedding(_num_stages * 2);
    umappp::Umap x;
    std::vector<double> data(_num_stages * 2);
    for(int i = 0; i < (_num_stages * 2); i++)
    {
         data[i] = 0;
    }
    x.set_num_neighbors(5).set_num_epochs(500);
    x.run(distance_umap, 2, embedding.data());
    int j =0;
    for(int x = 0; x < (int)embedding.size(); x+=2)
    {
        _scatter_points[j] = QPointF(embedding[x], embedding[x+1]);
        j++;
    }

    _axis_val = {std::numeric_limits<float>::infinity(),
                 -std::numeric_limits<float>::infinity(),
                 std::numeric_limits<float>::infinity(),
                 -std::numeric_limits<float>::infinity()}; //xmin,xmax, ymin,ymax
    for(int i=0;i<_scatter_points.size(); i++){
        if(_scatter_points[i].x() > _axis_val[1]){
            _axis_val[1] = _scatter_points[i].x();
        } else if(_scatter_points[i].x() < _axis_val[0]){
            _axis_val[0] = _scatter_points[i].x();
        }
        if(_scatter_points[i].y() > _axis_val[3]){
            _axis_val[3] = _scatter_points[i].y();
        } else if(_scatter_points[i].y() < _axis_val[2]){
            _axis_val[2] = _scatter_points[i].y();
        }
    }
    _axis_val[0] -= abs(0.1*_axis_val[0]);
    _axis_val[2] -= abs(0.1*_axis_val[2]);
    _axis_val[1] += abs(0.1*_axis_val[1]);
    _axis_val[3] += abs(0.1*_axis_val[3]);
    double per_point_x = width_height / (_axis_val[1] - _axis_val[0]) ;
    double per_point_y = width_height / (_axis_val[3] - _axis_val[2]);
    for(int i=0;i<_scatter_points.size(); i++){
        double value = 255* ((1.0*i) / (1.0*_scatter_points.size()));
        dots* dot = new dots(10,QColor(value,value,value, 175), i);
        dot->setToolTip("Stage: " + QString::number(i));
        dot->setCursor(Qt::ArrowCursor);
        dot->setFlag(QGraphicsItem::ItemIsSelectable);
        dot->setPos((_scatter_points[i].x()-_axis_val[0])*per_point_x, width_height -
                (_scatter_points[i].y()-_axis_val[2])*per_point_y);
        _scene_scatter->addItem(dot);
        connect(dot,SIGNAL(dotClicked(int)),this,SLOT(dot_clicked(int)));
        connect(dot,SIGNAL(dotHoverTrue(int)),this,SLOT(dot_hover_true(int)));
        connect(dot,SIGNAL(dotHoverFalse(int)),this,SLOT(dot_hover_false(int)));
        _dots_scatter_list.append(dot);
        _dots_high.append(false);
    }

}

void MainWindow::redraw_scatter()
{
    double width_height = 200;
    double height_offset = 0;
    double per_point_x = width_height / (_axis_val[1] - _axis_val[0]);
    double per_point_y = width_height / (_axis_val[3] - _axis_val[2]);
    for(int i=0;i< _dots_scatter_list.size();i++){
        dots* item = _dots_scatter_list[i];
        _scene->removeItem(item);
        delete item;
    }
    _dots_scatter_list.clear();
    _colors_dots.clear();


    for(int i=0;i<_scatter_points.size(); i++){
        double value = 1.0;
        QColor c_dot;
        if(_color_scat_time){
            value = 255*((1.0*i) / (1.0*_scatter_points.size()));
            c_dot = QColor(value,value,value, 175);
        } else {
            double diff_x = 100 / abs(_axis_val[1] - _axis_val[0]);
            double diff_y = 100 / abs(_axis_val[3] - _axis_val[2]);

            double x_c = fmax(((_scatter_points[i].x() - _axis_val[0]) * diff_x)-1, 0);
            double y_c = fmax(((_scatter_points[i].y() - _axis_val[2]) * diff_y)-1, 0);

            c_dot = QColor::fromString(twod_colors[int(x_c)][int(y_c)]);
            c_dot = QColor(c_dot.red(), c_dot.green(), c_dot.blue(), 175);
            _colors_dots.append(c_dot);

        }

        dots* dot = new dots(10,c_dot, i);
        dot->setToolTip("Stage: " + QString::number(i));
        dot->setCursor(Qt::ArrowCursor);
        dot->setFlag(QGraphicsItem::ItemIsSelectable);
        dot->setPos((_scatter_points[i].x()-_axis_val[0])*per_point_x, width_height -
                (_scatter_points[i].y()-_axis_val[2])*per_point_y + height_offset);
        _scene_scatter->addItem(dot);
        connect(dot,SIGNAL(dotClicked(int)),this,SLOT(dot_clicked(int)));
        connect(dot,SIGNAL(dotHoverTrue(int)),this,SLOT(dot_hover_true(int)));
        connect(dot,SIGNAL(dotHoverFalse(int)),this,SLOT(dot_hover_false(int)));
        _dots_scatter_list.append(dot);
    }

    for(int i=0; i< _dots_high.size(); i++){
        if(_dots_high[i]){
           _dots_scatter_list[i]->high_after_box_sel(true);
        }
    }
}

qreal MainWindow::calc_width_stages(QList<QString> names, QList<int> names_numbers, QString stage)
{
     qreal width =0;
     qreal width_stages = _width;
     qreal w_open_stage = (width_stages - _width_closed_rect*(_num_stages-names_numbers.size()))/(qreal) names.size();
     QVector<QString> text;
     QVector<QString> text_name;
     text = stage.mid(1).split("-");
     int min = text[0].toInt();
     int max = -1;
     int open_s = 0;
     if(text.size() > 1){
         max = text[1].toInt();
     }
     //three options: above, below or neither
     //eg. open stages S0 and S3-6
     //neither is S1-2 or S1
     //above is S0-7 or 1-7
     //below is S4-5 or S4
     //neither
     bool neither = true;
     for(int i=0; i< names_numbers.size(); i++){
         if(text.size() == 1){
             if(names_numbers[i] >= min && names_numbers[i] <= min){
                neither = false;
                break;
             }
         } else if (text.size() > 1){
             if(names_numbers[i] >= min && names_numbers[i] <= max){
                neither = false;
                break;
             }
         }
     }
     if(neither){
         if (text.size() == 1){
             width = _width_closed_rect;
         } else if (text.size() > 1){
             width = (max-min +1) * _width_closed_rect;
         }
     } else {
         //above the open stages, above always has min and max
         if(text.size() > 1){
             for(int i=0; i< names.size(); i++){
                 text_name = names[i].mid(1).split("-");
                 if(text_name.size() == 1){
                     if(text_name[0].toInt() >= min && text_name[0].toInt() <= max){
                         open_s ++;
                     }
                 } else if(text_name.size() > 1){
                     if(text_name[0].toInt() >= min && text_name[1].toInt() <= max){
                         open_s ++;
                     }
                 }
             }
         }
         //above true
         if(open_s > 0){
            int rest = 0;
            for(int p=min; p <= max; p++){
                bool num_in_list = false;
                for(int q=0; q < names_numbers.size(); q++){
                    if(names_numbers[q] == p){
                        num_in_list = true;
                        break;
                    }
                }
                if(!num_in_list){
                    rest ++;
                }
            }
            width = (qreal) open_s*w_open_stage + _width_closed_rect*rest;
         } else {
             //below true
             int min_name =0;
             int max_name = 0;
             bool below = false;
             for(int i=0; i< names.size(); i++){
                 //open stage must have size >1
                 if(names[i].mid(1).split("-").size() > 1){
                     min_name = names[i].mid(1).split("-")[0].toInt();
                     max_name = names[i].mid(1).split("-")[1].toInt();
                     if(max < 0){
                         if(min >= min_name && min <= max_name){
                             below = true;
                             break;
                         }
                     } else {
                         if(min >= min_name && max <= max_name){
                             below = true;
                             break;
                         }
                     }
                 }
             }
             if(below){
                 if(text.size() == 1){
                     width = w_open_stage / (qreal)(max_name-min_name+1);
                 } else if(text.size() > 1){
                     width = (max-min+1) * (w_open_stage / (qreal)(max_name-min_name+1));
                 }
             }
         }
     }
     return width;

}

void MainWindow::drawStages()
{

   _freq_per_stage.resize(_num_stages);
   _pat_per_stage.resize(_num_stages);
   _pat_per_stage_seq.clear();
   _pat_per_stage_seq.resize(_num_stages);
   _seq_data_stages.resize(_num_stages);
   std::set<QString> labels;
   for(int i=0; i< _seq_data_stages.size();i++){
      _seq_data_stages[i].resize(_num_seqs);
   }

   //get all category valuse
   for (int j =0; j < _data->rowCount(); j++){
       labels.insert(_data->item(j,1)->text());
   }

   //initialize freq per cat per stage
   for (int j =0; j < _num_stages; j++){
       for (auto itr : labels)
         {
            _freq_per_stage[j][itr] = 0;
            if(j ==0){
                _total_freqs[itr] = 0;
            }
         }
   }
   //initialize pat freq per stage
   if(_mode_staging_global == "patterns"){
       _num_pat_stage.clear();
       _num_pat_stage.resize(_num_stages);
       for (int j =0; j < _num_stages; j++){
           _pat_per_stage_seq[j].resize(_num_seqs);
           for (int k=0; k< _unique_pat.size(); k++)
             {
                _pat_per_stage[j][_unique_pat[k]] = 0;
             }
       }
       QList<int> stage_ind_seq;
       for(int j=0; j< _num_seqs; j++){
           stage_ind_seq.append(0);
       }
       for(int k=0; k<_patterns.size(); k++){
           int seq = _patterns[k][1].toInt();
           bool current_stage = false;
           //pattern in current stage
            if(stage_ind_seq[seq] <= _num_stages-2){
               if(stage_ind_seq[seq] == _num_stages-2 && _patterns[k][2].toInt() >= _stages_data[seq][_num_stages-2][0]){
                   current_stage = true;
                   stage_ind_seq[seq] ++;
               } else if (_patterns[k][2].toInt() < _stages_data[seq][stage_ind_seq[seq]][0] && _patterns[k][3].toInt() <= _stages_data[seq][stage_ind_seq[seq]][0]){
                   current_stage = true;
               }
           }
           if(current_stage || stage_ind_seq[seq] == _num_stages-1){
               if(_pat_per_stage[stage_ind_seq[seq]].contains(_patterns[k][0])){
                   _pat_per_stage[stage_ind_seq[seq]][_patterns[k][0]] ++;
                   _pat_per_stage_seq[stage_ind_seq[seq]][_patterns[k][1].toInt()][_patterns[k][0]] ++;
               } else{
                   _pat_per_stage[stage_ind_seq[seq]][_patterns[k][0]] = 1;
                   _pat_per_stage_seq[stage_ind_seq[seq]][_patterns[k][1].toInt()][_patterns[k][0]] = 1;
               }               
                _num_pat_stage[stage_ind_seq[seq]] ++;
           }

           //check if we need to go to next stage
           if(!current_stage && stage_ind_seq[seq] < _num_stages-2){
               while(_patterns[k][2].toInt() >= _stages_data[seq][stage_ind_seq[seq]][0]){
                   stage_ind_seq[seq] ++;
                   if(stage_ind_seq[seq] == _num_stages-2){
                       break;
                   }
               }
               if(_patterns[k][3].toInt() <= _stages_data[seq][stage_ind_seq[seq]][0]){
                   if(_pat_per_stage[stage_ind_seq[seq]].count(_patterns[k][0])){
                       _pat_per_stage[stage_ind_seq[seq]][_patterns[k][0]] ++;
                       _pat_per_stage_seq[stage_ind_seq[seq]][_patterns[k][1].toInt()][_patterns[k][0]] ++;
                   } else{
                       _pat_per_stage[stage_ind_seq[seq]][_patterns[k][0]] = 1;
                       _pat_per_stage_seq[stage_ind_seq[seq]][_patterns[k][1].toInt()][_patterns[k][0]] = 1;
                   }
               }
               _num_pat_stage[stage_ind_seq[seq]] ++;
           }
       }

   }

   //count per stage
   int seq = 0;
   int stage_in_seq = 0;
   int stage_in_seq_old = 0;
   bool first_seq = true;
   int length_1_seq = 0;
   QString prev_val = _data->item(0,0)->text();
   QList<QPair<QString, int>> seq_data_stage;
   QPair<QString, int> pair;
   for (int j =0; j < _data->rowCount(); j++){
       _total_freqs[_data->item(j,1)->text()] ++;
       if(prev_val != _data->item(j,0)->text())
       {           
           first_seq = false;
           prev_val = _data->item(j,0)->text();
           seq++;
           stage_in_seq = 0;           
       }
       if (first_seq){
           length_1_seq ++;
       }

       if (stage_in_seq < _stages_data[seq].size()) //also take last stage into account
       {
           if ((j - (seq*length_1_seq)) >= _stages_data[seq][stage_in_seq][0] && stage_in_seq < _num_stages -1){
               stage_in_seq ++;
           }
       }

       if(stage_in_seq != stage_in_seq_old)
       {
           if(stage_in_seq_old == _num_stages -1){
               _seq_data_stages[stage_in_seq_old][seq-1] = seq_data_stage;
           } else {
               _seq_data_stages[stage_in_seq_old][seq] = seq_data_stage;
           }
           seq_data_stage.clear();
           stage_in_seq_old = stage_in_seq;
       }

       _freq_per_stage[stage_in_seq][_data->item(j,1)->text()] ++;
       pair.first = _data->item(j,1)->text();
       pair.second = j - seq*length_1_seq;
       seq_data_stage.append(pair);
       if(_lowest_ind_stage.size() -1 < stage_in_seq){
            _lowest_ind_stage.append(j - seq*length_1_seq);
       } else{
           if (_lowest_ind_stage[stage_in_seq] > j - seq*length_1_seq){
              _lowest_ind_stage[stage_in_seq] = j - seq*length_1_seq;
           }
       }
   }
   //last part of seq in last stage
   _seq_data_stages[stage_in_seq_old][seq] = seq_data_stage;
   if (_lowest_ind_stage[stage_in_seq] > _data->rowCount()-1 - seq*length_1_seq){
      _lowest_ind_stage[stage_in_seq] = _data->rowCount()-1 - seq*length_1_seq;
   }

   if(_mode_staging_global == "patterns"){
       for(int j=0; j<_num_stages;j++){
           for(auto i = _pat_per_stage[j].cbegin(), end = _pat_per_stage[j].cend(); i != end; ++i)
           {
              if(_max_val < i.value())
              {
                _max_val = i.value();
              }
           }
       }

   } else {
       for(int j=0; j<_num_stages;j++){
           for(auto i = _freq_per_stage[j].cbegin(), end = _freq_per_stage[j].cend(); i != end; ++i)
           {
              if(_max_val < i.value())
              {
                _max_val = i.value();
              }
           }
       }
    }
   //ui->stages->centerOn(_stage_list[0]);
}

void MainWindow::make_hier_tree(int current_split, QString text_parent, int count, int level, QVector<QVector<double>> data, int max_height){
    QStringList tmp;
    QString text_left, text_right = "";
    QString new_parent_text_left, new_parent_text_right = "";
    int current_stage = 0;
    int offset_right = 0;
    if(text_parent == "done")
    {
      text_left = "done";
      new_parent_text_left = "done";
      text_right = "done";
      new_parent_text_right = "done";
    } else {
        tmp = text_parent.split("-");
        for(int i =0; i < data.size(); i++){            
            if (data[i][2] == current_split){
                current_stage = i + tmp[0].mid(1).toInt();
            }
        }
        if(current_stage  !=  tmp[0].mid(1).toInt()){
            text_left = tmp[0] + "-" + QString::number(current_stage);
            new_parent_text_left = text_left;

        } else {
            text_left = tmp[0];
            new_parent_text_left = "done";
        }

        if(current_stage+1 !=  tmp[1].toInt()){
            text_right = "S" + QString::number(current_stage+1) + "-" + tmp[1];
            new_parent_text_right = text_right;
            offset_right = current_stage - tmp[0].mid(1).toInt();

        } else {
            text_right = "S" + tmp[1];
            new_parent_text_right = "done";
            offset_right = data.size()-1;
        }
    }

    if (_levels.size() <= level+1){ //first time in level
        QVector<QString> tmp;
        tmp.append(text_left);
        tmp.append(text_right);
        _levels.append(tmp);
    } else {
        _levels[level+1].append(text_left);
        _levels[level+1].append(text_right);
    }
    if(count < max_height){
        make_hier_tree(current_split+1, new_parent_text_left, count+1, level+1, data.mid(0,offset_right), max_height);
        make_hier_tree(current_split+1, new_parent_text_right, count+1, level+1, data.mid(offset_right+1), max_height);
    }
}

void MainWindow::stage_tree()
{

    QString text_parent = "";
    text_parent = "S0-" + QString::number(_num_stages-1);
    QVector<QString> root;
    root.append(text_parent);
    _levels.append(root);
    int max_heigt = 0;
    for (int i=0; i< _stages_data[0].size(); i++){
        if (max_heigt < _stages_data[0][i][2]){
            max_heigt = _stages_data[0][i][2];
        }
    }
    //  list for each tree level
    make_hier_tree(0, text_parent, 0, 0, _stages_data[0], max_heigt+1);
    // delete levels that are too much
    for(int i = _levels.size()-1; i >= 0; i--){
        bool all_done = true;
        for(int j=0; j<_levels[i].size(); j++){
            if(_levels[i][j] != "done"){
                all_done = false;
            }
        }
        if(all_done){
           _levels.remove(i);
        }
    }
    for(int i = 0; i < _levels.size(); i++){
        for(int j=0; j<_levels[i].size(); j++){
           _stage_tree.append(_levels[i][j]);
        }
    }

}

void MainWindow::cluster_seqs_stages(){
    std::vector<std::string> cats_vec;
    std::vector<bool> cats_filter_vec;
    for(int i =0; i< _cats.size();i++){
       cats_vec.push_back(_cats[i].toStdString());
    }
    for(int i =0; i< _cats_filter.size();i++){
       cats_filter_vec.push_back(_cats_filter[i]);
    }
   _len_seq = _data->rowCount()/_num_seqs;
   for(int i=0; i < _num_stages; i++){
        QList<int> start_time_ind;
        QList<int> end_time_ind;
        std::vector<std::vector<std::string>> seqs;
        seqs.resize(_seq_data_stages[i].size());
        QList<QMap<QString, double>> total_counts_per_seq;
        total_counts_per_seq.resize(_seq_data_stages[i].size());
        for(int j = 0; j < _seq_data_stages[i].size(); j++){ //loop over all seqs in stage
            for(int k=0; k<_seq_data_stages[i][j].size(); k++){ //loop over one seq
                seqs[j].push_back(_seq_data_stages[i][j][k].first.toStdString());
                //get dict with freq for each stage
                if(total_counts_per_seq[j].count(_seq_data_stages[i][j][k].first)){
                    total_counts_per_seq[j][_seq_data_stages[i][j][k].first] ++;
                } else{
                    total_counts_per_seq[j][_seq_data_stages[i][j][k].first] = 1;
                }
                if(k == 0){
                    start_time_ind.append(_seq_data_stages[i][j][k].second);
                }
                if(k == _seq_data_stages[i][j].size()-1){
                    end_time_ind.append(_seq_data_stages[i][j][k].second);
                }
            }
        }
        //get dist metric of seqs
        QList<QList<double>> dist_metr;
        dist_metr.resize(seqs.size());

        for(int j=0;j<seqs.size(); j++){
            for(int k=0;k<seqs.size(); k++){
                if(k <= j){
                    if (k == j){
                        dist_metr[j].append(0);
                    } else {                        
                        std::unordered_map<std::string, double> counts_s1_map;
                        std::unordered_map<std::string, double> counts_s2_map;
                        for (auto k = total_counts_per_seq[j].cbegin(), end = total_counts_per_seq[j].cend(); k != end; ++k){
                            counts_s1_map.insert({k.key().toStdString(), k.value()});
                        }
                        for (auto l = total_counts_per_seq[k].cbegin(), end = total_counts_per_seq[k].cend(); l != end; ++l){
                            counts_s2_map.insert({l.key().toStdString(), l.value()});
                        }
                        double dist = dist_two_seq(counts_s1_map, counts_s2_map, cats_vec, cats_filter_vec);
                        dist_metr[j].append(dist);
                        dist_metr[k].append(dist);
                    }
                }
            }
        }
        //get clusters
        auto dbscan = DBSCAN<std::vector<std::string>, float>();
        dbscan.Run(&seqs, seqs[0].size(), _eps_clusters_tree, 2, dist_metr);
        auto noise = dbscan.Noise;
        auto clusters = dbscan.Clusters;
        std::map<int, QList<int>> result;
        int count = 0;
        for(int j=0; j< clusters.size(); j++){
            QVector<int> vector = QVector<int>(clusters[j].begin(), clusters[j].end());
            result[count] = QList<int>::fromVector(vector);
            count ++;
        }
        for(int j=0; j< noise.size(); j++){
            QList<int> l = {0};
            l[0] = noise[j];
            result[count] = l;
            count ++;
        }
        _cluster_info.append(result);

        //info about start and end time
        QList<QString> times;
        for(int j=0; j< start_time_ind.size(); j++){
            times.append(_data->item(start_time_ind[j]+j*_len_seq,2)->text().mid(5));
            times.append(_data->item(end_time_ind[j]+j*_len_seq,2)->text().mid(5));
        }
        std::sort(std::begin(times), std::end(times));
        _start_end_time_stage[i] = qMakePair(times[0].mid(0,5) + " " + times[0].mid(6,8),
                times[times.size()-1].mid(0,5) + " " + times[times.size()-1].mid(6,8));        
    }
}

MainWindow::~MainWindow()
{
      qDebug() << "done";
    delete ui;
}


void MainWindow::box_selection(bool selection){
    if(selection)
    {
      ui->scatter->setDragMode(QGraphicsView::RubberBandDrag);
    } else {
      QList<QGraphicsItem*> lijst = ui->scatter->items(ui->scatter->rubberBandRect());
      //remove previous box highlight
      if(lijst.size() > 1){
          for(int i =0; i < _sel_dots.size();i++){
             _dots_scatter_list[_sel_dots[i]]->high_after_box_sel(false);
             _dots_high[_sel_dots[i]] = false;
          }

          for(int i=0;i<_levels.size();i++){
              for(int j=0;j<_levels[i].size();j++){
                for(int q=0; q < _sel_dots.size(); q++){
                   if(_levels[i][j] == "S" + QString::number(_sel_dots[q])){
                      _stage_status[i][j] = "false";
                      break;
                   }
                }
              }
          }
      }
      //add new highlight
      _sel_dots.clear();      
      for(int i =0; i < lijst.size()-1;i++){ //last item is axes and background
         dots* dot = qgraphicsitem_cast<dots*>(lijst[i]);
         _sel_dots.append(dot->getIndex());
         if(_dots_high[dot->getIndex()] != true){
             dot->high_after_box_sel(true);
         }
         _dots_high[dot->getIndex()] = true;
      }
      ui->scatter->setDragMode(QGraphicsView::ScrollHandDrag);
      //add highlight to tree view
      QVector<double> changed;
      if(_start){
          for(int i=0;i<_levels.size();i++){
              for(int j=0;j<_levels[i].size();j++){
                if(_stage_status[i][j] == "true"){
                    _stage_status[i][j] = "false";
                }
                for(int q=0; q < _sel_dots.size(); q++){
                   if(_levels[i][j] == "S" + QString::number(_sel_dots[q])){
                      _stage_status[i][j] = "true";
                      break;
                   }
                }
                if(_levels[i][j] == "S" + QString::number(_sel_dots[0])){
                    changed.append(i);
                    changed.append(j);
                }
              }
          }
          _start = false;
      } else {
          QVector<QVector<double>> changed_list;
          QVector<double> changed_tmp;
          for(int i=0;i<_levels.size();i++){
              for(int j=0;j<_levels[i].size();j++){
                for(int q=0; q < _sel_dots.size(); q++){
                   if(_levels[i][j] == "S" + QString::number(_sel_dots[q])){
                      _stage_status[i][j] = "true";
                      changed_tmp.append(i);
                      changed_tmp.append(j);
                      changed_list.append(changed_tmp);
                      break;
                   }
                }
              }
          }
          if(changed_list.size() > 0){
            changed = changed_list[0];
          } else {
              changed = {0,0};
          }
          if (changed_list.size() > 1){
              for(int i=1;i< changed_list.size();i++){
                  //if parent of changed is open close it
                  int level = changed_list[i][0];
                  double index = changed_list[i][1];
                  while (level > 0){
                     level -= 1;
                     index = ceil((index +1)/2)-1;
                     if(_stage_status[level][index] == "true"){
                        _stage_status[level][index] = "false";
                     }
                 }

                  //if child of changed is open close it
                  level = changed_list[i][0];
                  index = changed_list[i][1];
                  int index2 = changed_list[i][1];
                  while(level < _levels.size()-1){
                      level ++;
                      index = index*2;
                      index2 = index2*2 +1;
                      for(int f= index; f <= index2; f++){
                          if(_stage_status[level][f] == "true"){
                             _stage_status[level][f] = "false";
                          }
                      }
                  }
              }
          }
      }
      if(_sel_dots.size() > 0){
          _stage_rects_old = _stage_rects;
          redraw_tree(changed, false, true);

          QParallelAnimationGroup* anim_group = new QParallelAnimationGroup;
          for(int i=0;i< _levels.size();i++){
              for(int j=0; j< _levels[i].size(); j++){
                  if(_levels[i][j] != "done"){
                     double xold = _stage_rects_old[i][j]->pos().x();
                     double yold = _stage_rects_old[i][j]->pos().y();
                     double xnew = _stage_rects[i][j]->pos().x();
                     double ynew = _stage_rects[i][j]->pos().y();
                     double wold = _stage_rects_old[i][j]->boundingRect().width();
                     double hold = _stage_rects_old[i][j]->boundingRect().height();
                     double wnew = _stage_rects[i][j]->boundingRect().width();
                     double hnew = _stage_rects[i][j]->boundingRect().height();

                     if(wold != wnew || hold!= hnew){
                         QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "");
                         anim->setStartValue(QPointF(1,1));
                         anim->setEndValue(QPointF(wnew/wold,hnew/hold));
                         anim->setDuration(1000);
                         anim->setEasingCurve(QEasingCurve::OutCurve);
                         connect(anim,SIGNAL(valueChanged(QVariant)), _stage_rects_old[i][j],SLOT(setScaleXY(QVariant)));
                         anim_group->addAnimation(anim);
                     }

                     if(xold != xnew || yold!= ynew){
                         QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "pos");
                         anim->setStartValue(QPointF(xold,yold));
                         anim->setEndValue(QPointF(xnew,ynew));
                         anim->setDuration(1000);
                         anim->setEasingCurve(QEasingCurve::OutCurve);
                         anim_group->addAnimation(anim);
                     }
                  }
              }
          }
          connect(anim_group,SIGNAL(finished()), this,SLOT(finished_animation()));
          anim_group->start();
      }
    }
}


void MainWindow::stage_rect_clicked(QVector<double> index){

    QString status = _stage_status[index[0]][index[1]];
    bool open = true;

    if (status == "true"){
        _stage_status[index[0]][index[1]] = "false";
        open = false;
    } else if (status == "false") {
       _stage_status[index[0]][index[1]] = "true";
       open = true;
    }

    QString name_total = _levels[index[0]][index[1]];

    int min = name_total.mid(1).split("-")[0].toInt();
    int max = -1;
    if(name_total.mid(1).split("-").size() > 1){
        max = name_total.mid(1).split("-")[1].toInt();
    }

    //if parent is open, put these dots on non high
    QString parent = "none";
    int level = index[0];
    double ind = index[1];
    while (level > 0){
       level -= 1;
       ind = ceil((ind +1)/2)-1;
       if(_stage_status[level][ind] == "true"){
          parent = _levels[level][ind];
       }
    }
    if(parent != "none"){
        int min_p = parent.mid(1).split("-")[0].toInt();
        int max_p = -1;
        if(parent.mid(1).split("-").size() > 1){
            max_p = parent.mid(1).split("-")[1].toInt();
        }
        dots* dot_p = _dots_scatter_list[min_p];
        _dots_high[min_p] = false;
        dot_p->high_after_box_sel(false);
        if(max_p > -1){
            for(int i = min_p+1; i <= max_p; i++){
                dots* dot2_p = _dots_scatter_list[i];
                _dots_high[i] = false;
                dot2_p->high_after_box_sel(false);
            }
        }
    }

    dots* dot = _dots_scatter_list[min];
    if(_dots_high[min] == false && open){
        _dots_high[min] = true;
        dot->high_after_box_sel(true);
    } else if(_dots_high[min] == true && !open){
        _dots_high[min] = false;
        dot->high_after_box_sel(false);
    }
    if(max > -1){
        for(int i = min+1; i <= max; i++){
            dots* dot2 = _dots_scatter_list[i];
            if(_dots_high[i] == false && open){
                _dots_high[i] = true;
                dot2->high_after_box_sel(true);
            } else if(_dots_high[i] == true && !open){
                _dots_high[i] = false;
                dot2->high_after_box_sel(false);
            }
        }
    }

    _stage_rects_old = _stage_rects;
    redraw_tree(index, false, true);

    QParallelAnimationGroup* anim_group = new QParallelAnimationGroup;
    for(int i=0;i< _levels.size();i++){
        for(int j=0; j< _levels[i].size(); j++){
            if(_levels[i][j] != "done"){
               double xold = _stage_rects_old[i][j]->pos().x();
               double yold = _stage_rects_old[i][j]->pos().y();
               double xnew = _stage_rects[i][j]->pos().x();
               double ynew = _stage_rects[i][j]->pos().y();
               double wold = _stage_rects_old[i][j]->boundingRect().width();
               double hold = _stage_rects_old[i][j]->boundingRect().height();
               double wnew = _stage_rects[i][j]->boundingRect().width();
               double hnew = _stage_rects[i][j]->boundingRect().height();

               if(wold != wnew || hold!= hnew){
                   QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "");
                   anim->setStartValue(QPointF(1,1));
                   anim->setEndValue(QPointF(wnew/wold,hnew/hold));
                   anim->setDuration(1000);
                   anim->setEasingCurve(QEasingCurve::OutCurve);
                   connect(anim,SIGNAL(valueChanged(QVariant)), _stage_rects_old[i][j],SLOT(setScaleXY(QVariant)));
                   anim_group->addAnimation(anim);
               }

               if(xold != xnew || yold!= ynew){
                   QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "pos");
                   anim->setStartValue(QPointF(xold,yold));
                   anim->setEndValue(QPointF(xnew,ynew));
                   anim->setDuration(1000);
                   anim->setEasingCurve(QEasingCurve::OutCurve);
                   anim_group->addAnimation(anim);
               }
            }
        }
    }
    connect(anim_group,SIGNAL(finished()), this,SLOT(finished_animation()));
    anim_group->start();
}

void MainWindow::finished_animation(){
    for(int i=0;i< _levels.size();i++){
        for(int j=0; j< _levels[i].size(); j++){
            if(_levels[i][j] != "done"){
                tree* item = _stage_rects_old[i][j];
                _scene->removeItem(item);
                delete item;
                _scene->addItem(_stage_rects[i][j]);
            }
        }
    }
}

void MainWindow::dot_clicked(int index){

    if(_dots_high[index] == true){
        _dots_high[index] = false;
    } else {
        _dots_high[index] = true;
    }


    if(_start){
        QVector<double> changed;
        for(int i=0;i<_levels.size();i++){
            for(int j=0;j<_levels[i].size();j++){
              if(_levels[i][j] != "S" + QString::number(index) &&
                      _stage_status[i][j] == "true"){
                  _stage_status[i][j] = "false";
              }
              if(_levels[i][j] == "S" + QString::number(index)){
                  changed.append(i);
                  changed.append(j);
              }
            }
        }
        _start = false;
        _stage_rects_old = _stage_rects;
        redraw_tree(changed, false, true);

        QParallelAnimationGroup* anim_group = new QParallelAnimationGroup;
        for(int i=0;i< _levels.size();i++){
            for(int j=0; j< _levels[i].size(); j++){
                if(_levels[i][j] != "done"){
                   double xold = _stage_rects_old[i][j]->pos().x();
                   double yold = _stage_rects_old[i][j]->pos().y();
                   double xnew = _stage_rects[i][j]->pos().x();
                   double ynew = _stage_rects[i][j]->pos().y();
                   double wold = _stage_rects_old[i][j]->boundingRect().width();
                   double hold = _stage_rects_old[i][j]->boundingRect().height();
                   double wnew = _stage_rects[i][j]->boundingRect().width();
                   double hnew = _stage_rects[i][j]->boundingRect().height();

                   if(wold != wnew || hold!= hnew){
                       QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "");
                       anim->setStartValue(QPointF(1,1));
                       anim->setEndValue(QPointF(wnew/wold,hnew/hold));
                       anim->setDuration(1000);
                       anim->setEasingCurve(QEasingCurve::OutCurve);
                       connect(anim,SIGNAL(valueChanged(QVariant)), _stage_rects_old[i][j],SLOT(setScaleXY(QVariant)));
                       anim_group->addAnimation(anim);
                   }

                   if(xold != xnew || yold!= ynew){
                       QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "pos");
                       anim->setStartValue(QPointF(xold,yold));
                       anim->setEndValue(QPointF(xnew,ynew));
                       anim->setDuration(1000);
                       anim->setEasingCurve(QEasingCurve::OutCurve);
                       anim_group->addAnimation(anim);
                   }
                }
            }
        }
        connect(anim_group,SIGNAL(finished()), this,SLOT(finished_animation()));
        anim_group->start();
    } else {
        QVector<double> changed;
        for(int i=0;i<_levels.size();i++){
            for(int j=0;j<_levels[i].size();j++){
              if(_levels[i][j] == "S" + QString::number(index)){
                  changed.append(i);
                  changed.append(j);
                  if (_dots_high[index]){
                      _stage_status[i][j] = "true";
                  } else {
                      _stage_status[i][j] = "false";
                  }
              }
            }
        }
        _stage_rects_old = _stage_rects;
        redraw_tree(changed, false, true);

        QParallelAnimationGroup* anim_group = new QParallelAnimationGroup;
        for(int i=0;i< _levels.size();i++){
            for(int j=0; j< _levels[i].size(); j++){
                if(_levels[i][j] != "done"){
                   double xold = _stage_rects_old[i][j]->pos().x();
                   double yold = _stage_rects_old[i][j]->pos().y();
                   double xnew = _stage_rects[i][j]->pos().x();
                   double ynew = _stage_rects[i][j]->pos().y();
                   double wold = _stage_rects_old[i][j]->boundingRect().width();
                   double hold = _stage_rects_old[i][j]->boundingRect().height();
                   double wnew = _stage_rects[i][j]->boundingRect().width();
                   double hnew = _stage_rects[i][j]->boundingRect().height();

                   if(wold != wnew || hold!= hnew){
                       QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "");
                       anim->setStartValue(QPointF(1,1));
                       anim->setEndValue(QPointF(wnew/wold,hnew/hold));
                       anim->setDuration(1000);
                       anim->setEasingCurve(QEasingCurve::OutCurve);
                       connect(anim,SIGNAL(valueChanged(QVariant)), _stage_rects_old[i][j],SLOT(setScaleXY(QVariant)));
                       anim_group->addAnimation(anim);
                   }

                   if(xold != xnew || yold!= ynew){
                       QPropertyAnimation* anim = new QPropertyAnimation( _stage_rects_old[i][j], "pos");
                       anim->setStartValue(QPointF(xold,yold));
                       anim->setEndValue(QPointF(xnew,ynew));
                       anim->setDuration(1000);
                       anim->setEasingCurve(QEasingCurve::OutCurve);
                       anim_group->addAnimation(anim);
                   }
                }
            }
        }
        connect(anim_group,SIGNAL(finished()), this,SLOT(finished_animation()));
        anim_group->start();
    }
}

void MainWindow::dot_hover_true(int ind){
    QVector<double> changed;
    for(int i=0;i<_levels.size();i++){
        for(int j=0;j<_levels[i].size();j++){
            if(_levels[i][j] == "S" + QString::number(ind)){
                _stage_high[i][j] = "true";
                changed.append(i);
                changed.append(j);
            }
        }
    }

    redraw_tree(changed, true, false);
}

void MainWindow::dot_hover_false(int ind){
     QVector<double> changed;
    for(int i=0;i<_levels.size();i++){
        for(int j=0;j<_levels[i].size();j++){
            if(_levels[i][j] == "S" + QString::number(ind)){
                _stage_high[i][j] = "false";
                changed.append(i);
                changed.append(j);
            }
        }
    }
    redraw_tree(changed, true, false);
}

void MainWindow::on_low_level_stateChanged(int arg1)
{
    if(arg1 == 2){
        ui->high_level->setChecked(false);
        ui->mid_level->setChecked(false);
        _mode = "low";
        QVector<double> index = {0,0};
        redraw_tree(index, true, false);
        qDebug() << "tree done";
    }
}

void MainWindow::on_mid_level_stateChanged(int arg1)
{
    if(arg1 == 2){

        ui->low_level->setChecked(false);
        ui->high_level->setChecked(false);
        _mode = "mid";
        QVector<double> index = {0,0};
        redraw_tree(index, true, false);
    }
}

void MainWindow::on_high_level_stateChanged(int arg1)
{
    if(arg1 == 2){

        ui->low_level->setChecked(false);
        ui->mid_level->setChecked(false);
        _mode = "high";
        QVector<double> index = {0,0};
        redraw_tree(index, true, false);
    }
}


void MainWindow::on_cluster_button_stateChanged(int arg1)
{
     if(arg1 == 2){
        _cluster = true;
        if(_mode != "high"){
            int row_margin = 2;
            QParallelAnimationGroup* anim_dwarf_willows = new QParallelAnimationGroup;
            for(int i=0;i< _levels.size();i++){
                for(int j=0; j< _levels[i].size(); j++){
                    if(_stage_status[i][j] == "true"){
                        for(int k=0;k<_dwarf_willows[i][j].size(); k++){ //list with all blocks per seq [[blocks seq 0],[block seq 1]]
                            int new_y = 0;
                            QList<int> key;
                            std::map<int, QList<int>> stage_clusters =
                                    _cluster_info[_levels[i][j].mid(1).split("-")[0].toInt()];
                            for(std::map<int,QList<int>>::iterator it = stage_clusters.begin(); it != stage_clusters.end(); ++it) {
                              key.append(it->first);
                            }
                            bool done = false;
                            for(int s=0; s< key.size();s++){
                                for(int u =0; u<stage_clusters[key[s]].size(); u++){
                                    if(stage_clusters[key[s]][u] == k){                                        
                                        done = true;
                                        break;
                                    }
                                    new_y += _block_size + row_margin;
                                }
                               if(done){
                                   break;
                               }
                               new_y += 10;
                            }
                            for (int l=0;l<_dwarf_willows[i][j][k].size(); l++){
                                QPropertyAnimation* anim = new QPropertyAnimation(_dwarf_willows[i][j][k][l] , "pos");
                                anim->setStartValue(_dwarf_willows[i][j][k][l]->pos());
                                anim->setEndValue(QPointF(_dwarf_willows[i][j][k][l]->pos().x(), new_y));
                                anim->setDuration(1000);
                                anim->setEasingCurve(QEasingCurve::OutCurve);
                                anim_dwarf_willows->addAnimation(anim);

                            }
                        }
                     }
                  }
            }
            connect(anim_dwarf_willows,SIGNAL(finished()), this,SLOT(finished_anim_clus()));
            anim_dwarf_willows->start();
        }


     } else{
       _cluster = false;
       _cluster_on = -1;
       ui -> spinBox_4 -> setValue(-1);
       if(_mode != "high"){
           int row_margin = 2;
           QParallelAnimationGroup* anim_dwarf_willows = new QParallelAnimationGroup;
           for(int i=0;i< _levels.size();i++){
               for(int j=0; j< _levels[i].size(); j++){
                   if(_stage_status[i][j] == "true"){
                       int new_y = 0;
                       for(int k=0;k<_dwarf_willows[i][j].size(); k++){ //list with all blocks per seq [[blocks seq 0],[block seq 1]
                           for (int l=0;l<_dwarf_willows[i][j][k].size(); l++){
                               QPropertyAnimation* anim = new QPropertyAnimation(_dwarf_willows[i][j][k][l] , "pos");
                               anim->setStartValue(_dwarf_willows[i][j][k][l]->pos());
                               anim->setEndValue(QPointF(_dwarf_willows[i][j][k][l]->pos().x(), new_y));
                               anim->setDuration(1000);
                               anim->setEasingCurve(QEasingCurve::OutCurve);
                               anim_dwarf_willows->addAnimation(anim);
                           }
                           new_y += _block_size + row_margin;
                       }
                    }
                 }
           }
           connect(anim_dwarf_willows,SIGNAL(finished()), this,SLOT(finished_anim_clus()));
           anim_dwarf_willows->start();
       }
     }
}

void MainWindow::finished_anim_clus()
{
    QVector<double> index = {0,0};
    redraw_tree(index, true, false);
}

void MainWindow::on_time_stateChanged(int arg1)
{
    if(arg1 == 2){
        _color_scat_time = true;
        ui->similarity->setChecked(false);
        QVector<double> index = {0,0};
        redraw_scatter();
        redraw_tree(index, true, false);        
    }
}


void MainWindow::on_similarity_stateChanged(int arg1)
{
    if(arg1 == 2){
        _color_scat_time = false;
        ui->time->setChecked(false);
        QVector<double> index = {0,0};
        redraw_scatter();
        redraw_tree(index, true, false);

    }
}


void MainWindow::on_doubleSpinBox_valueChanged(double arg1)
{
    _max_stages = arg1;
    qDebug() << _max_stages;
}

//num min events
void MainWindow::on_spinBox_2_valueChanged(int arg1)
{
    _min_events_stage = arg1;
}

//max dev
void MainWindow::on_spinBox_3_valueChanged(int arg1)
{
    _max_dev = arg1;
}

//global splitting
//entropy
void MainWindow::on_checkBox_2_stateChanged(int arg1)
{
    if(arg1 == 2){
        _mode_staging_global = "entropy";
        ui->checkBox_3->setChecked(false);
    } else {
        _mode_staging_global = "patterns";
        ui->checkBox_3->setChecked(true);
    }
}

//patterns
void MainWindow::on_checkBox_3_stateChanged(int arg1)
{
    if(arg1 == 2){
        _mode_staging_global = "patterns";
        ui->checkBox_2->setChecked(false);
    } else {
        _mode_staging_global = "entropy";
        ui->checkBox_2->setChecked(true);
    }

}

//local splitting
//entropy
void MainWindow::on_checkBox_4_stateChanged(int arg1)
{
    if(arg1 == 2){
        _mode_staging_local = "entropy";
        ui->checkBox_5->setChecked(false);
    } else {
        _mode_staging_local = "patterns";
        ui->checkBox_5->setChecked(true);
    }
}

//patterns
void MainWindow::on_checkBox_5_stateChanged(int arg1)
{
    if(arg1 == 2){
        _mode_staging_local = "pattern";
        ui->checkBox_4->setChecked(false);
    } else {
        _mode_staging_local = "entropy";
        ui->checkBox_4->setChecked(true);
    }

}

//rerun staging
void MainWindow::on_pushButton_clicked()
{
    qDebug() << "rerun";
    //delete all
    _scene->clear();
    _scene_scatter->clear();
    _stages_data.clear();
    _levels.clear();
    _stage_tree.clear();
    _freq_per_stage.clear();
    _stage_rects.clear();
    _stage_rects_old.clear();
    _stage_status.clear();
    _stage_high.clear();
    _scatter_points.clear();
    _sel_dots.clear();
    _dots_high.clear();
    _dots_scatter_list.clear();
    _start = true;
    _seq_data_stages.clear();
    _lowest_ind_stage.clear();
    _cluster_info.clear();
    _dwarf_willows.clear();
    _axis_val.clear();
    _colors_dots.clear();
    _total_freqs.clear();
    _patterns.clear();
    _pat_id_to_pat.clear();
    _unique_pat.clear();
    _most_freq_pat.clear();
    _total_pats.clear();
    _high_patterns_events.clear();
    _max_val = 0;
    _pan.clear();
    _parent_lows.clear();
    _max_sizes_seqs.clear();
    _zoom_mid_level.clear();
    _zoom.clear();
    //rerun

    qDebug() << "one";
    _num_seqs = _data->item(_data->rowCount()-1,0)->text().toInt() +1;
    if(_mode_staging_global == "patterns"){
        //pattern file should be sorted on start index
        QSet<QString> set_pat;
        QList<QString> tmp_path = QDir::currentPath().split("/");
        QString path = "";
        for(int i=0; i<tmp_path.size()-1; i++){
            path += tmp_path[i] + "/";
        }
        if(!_run_time_test){
            _pattern_path = path + "pattern_file.csv";
        }
        //The pattern file is loaded here. The different patterns can overlap.
        //There are no patterns of length one. We did some pre-processing on the pattern files:
        //- We only included the patterns that occured in total above a certain threshold.
        //- And in the pattern files we used (except for dummy) we excluded patterns with (many) gaps.
        QFile file(_pattern_path);
        QString Alldata;
        QStringList rowAsString;
        QStringList row;

        if (file.open(QFile::ReadOnly)){
               Alldata = file.readAll();
               rowAsString = Alldata.split("\n");
               file.close();
        }
        for (int i = 1; i < rowAsString.size(); i++){
               row = rowAsString.at(i).split(",");
               QList<QString> pat;
               if(row.size() > 1){
                  pat.append(row[0]);
                  pat.append(row[row.size()-3]);
                  pat.append(row[row.size()-2]);
                  pat.append(row[row.size()-1]);
                  _patterns.append(pat);
                  set_pat.insert(row[0]);
                  if(!_pat_id_to_pat.contains(row[0])){
                      QList<QString> pat;
                      pat.append(row[1].split("[")[1].replace("'", ""));
                      for(int q = 2; q<row.size()-5; q++){
                          QString tmp = row[q].replace("'", "");
                          pat.append(tmp.replace(" ", ""));
                      }
                      QString last = row[row.size()-5].split("]")[0];
                      last = last.replace("'", "");
                      last = last.replace(" ", "");
                      pat.append(last);
                      _pat_id_to_pat[row[0]] = pat;
                  }
                  if(!_total_pats.contains(row[0])){
                      _total_pats[row[0]] = 1;
                  } else {
                      _total_pats[row[0]] ++;
                  }
               }
        }
        for (auto i = set_pat.begin(), end = set_pat.end(); i != end; ++i){
            _unique_pat.append(*i);
        }
        QList<QPair<int,QString>> most_freq_pat_list;
        for (auto i = _total_pats.begin(), end = _total_pats.end(); i != end; ++i){
            most_freq_pat_list.append(qMakePair(i.value(), i.key()));
        }
        std::sort(std::begin(most_freq_pat_list), std::end(most_freq_pat_list));
        for(int i=most_freq_pat_list.size()-1; i >= 0; i--){
            if(i>most_freq_pat_list.size()-1-20){
                _most_freq_pat.append(most_freq_pat_list[i].second);
            }
        }
    }
    qDebug() << "two";
    QElapsedTimer timer;
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    timer.start();
    std::vector<std::string> cats_vec;
    std::vector<bool> cats_filter_vec;
    for(int i =0; i< _cats.size();i++){
       cats_vec.push_back(_cats[i].toStdString());
    }
    for(int i =0; i< _cats_filter.size();i++){
       cats_filter_vec.push_back(_cats_filter[i]);
    }
    std::unordered_map<std::string, double> total_counts_event_map;
    for (auto k = _total_count_event.cbegin(), end = _total_count_event.cend(); k != end; ++k){
        total_counts_event_map.insert({k.key().toStdString(), k.value()});
    }
    _stages_data = staging(_data, _num_seqs, _min_events_stage, _max_stages, _max_dev,
                           _mode_staging_global, 0, total_counts_event_map, ui->progressBar,
                           cats_vec, cats_filter_vec, _mode_staging_local, _run_time_test,
                           _pattern_path, _staging_eps);
    qDebug() << timer.elapsed();
    qDebug() << "total staging";
    ui->progressBar->setValue(100);
    ui->progressBar->setVisible(false);
    if(_stages_data.size() > 0){
        if(_stages_data[0].size() > 0){
            _num_stages = _stages_data[0].size() + 1;            
            qDebug() << "three";
            drawStages();
            qDebug() << "four";
            cluster_seqs_stages();
            stage_tree();
            qDebug() << "five";
            draw_tree();
            qDebug() << "six";
            draw_scatter();
            qDebug() << "seven";
        } else{
            QMessageBox msgBox;
            msgBox.setText("No stages discovered.");
            msgBox.exec();
        }
    } else{
        QMessageBox msgBox;
        msgBox.setText("No stages discovered.");
        msgBox.exec();
    }

}


void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    for(int i=0; i< _cats.size(); i++){
        if(_cats[i] == item->text()){
            _cats_filter[i] = !_cats_filter[i];
        }
    }
    _filtered_out_cats.clear();
    for(int i=0; i< _cats.size(); i++){
        if(!_cats_filter[i]){
            _filtered_out_cats.append(_cats[i]);
        }
    }
    redraw_tree({0,0},true,false);
}


void MainWindow::on_listWidget_2_itemClicked(QListWidgetItem *item)
{
    for(int i=0; i< _seq_info.values().size(); i++){
        if(_seq_info[i] == item->text()){
            _seqs_filter[i] = !_seqs_filter[i];
        }
    }
    redraw_tree({0,0},true,false);
}

void MainWindow::rect_bar_clicked(QString rect){
    bool same = false;
    if(_high_through_click.contains(rect)){
        same = true;
    }
    bool something_already_high = false;
    if(_high_through_click.size() > 0){
       something_already_high = true;
    }
    _high_through_click.clear();
    _high_patterns_events.clear();
    if(rect != "" && !same) {
       if(_mode_staging_global == "patterns"){
           _high_patterns_events.resize(_num_seqs);
           for(int i=0; i< _patterns.size(); i++){
               if(_patterns[i][0] == rect){
                   for(int j=_patterns[i][2].toInt(); j< _patterns[i][3].toInt(); j++){
                      _high_patterns_events[_patterns[i][1].toInt()].append(j);
                   }
               }
           }
       }
       _high_through_click.append(rect);

    }
    if(_high_through_click.size() > 0 || something_already_high){
        redraw_tree({0,0},true,false);
    }

}

void MainWindow::rect_event_clicked(QString rect){
    _high_patterns_events.clear();
    QList<QString> info = rect.split("/");
    double zoom = _zoom[info[3].toInt()][info[4].toInt()];
    //check if pattern is created
    if(_high_through_click.size()>0 && _prev_mouse.x()){
        if(abs(_prev_mouse.x() - info[1].toDouble()) < _block_size*2*zoom +_block_margin*zoom &&
                abs(_prev_mouse.y() - info[2].toDouble()) < _block_size*zoom){
            _pat_created = true;
            _high_through_click.append(info[0]);
        } else {
            _pat_created = false;
        }
    }
    if(!_pat_created){
        //if no pattern is created
        if(_high_through_click.contains(info[0])){
           _high_through_click.clear();
        } else {
            _high_through_click.clear();
            _high_through_click.append(info[0]);
        }
    }
    redraw_tree({0,0},true,false);
    _prev_mouse.setX(info[1].toDouble());
    _prev_mouse.setY(info[2].toDouble());
}


void MainWindow::seq_high_cluster(int i){
    if(_cluster && _mode != "high" && _hover_cluster_ind != i){
       _hover_cluster_ind = i;
       redraw_tree({0,0},true,false);
    }
}

void MainWindow::seq_low_cluster(int i){
    if(_cluster && _mode != "high"){
        _hover_cluster_ind = -1;
        for(int i=0; i< _stage_status.size(); i++){
            for(int j=0; j< _stage_status[i].size(); j++){
               if(_stage_status[i][j] == "true"){
                   for(int q=0; q<_dwarf_willows[i][j].size(); q++){
                       for(int p=0; p<_dwarf_willows[i][j][q].size(); p++){
                          _dwarf_willows[i][j][q][p]->update_hover();
                       }
                   }
               }
            }
        }
    }
}

void MainWindow::linked_zoom(QPair<double, QString> scale){
    if(_shared_zoom){
        for(int p=0; p < _levels.size(); p++){
            for(int q=0; q < _levels[p].size(); q++){
                if(_stage_status[p][q] == "true"){
                   _zoom[p][q] = scale.first;
                   _parent_lows[p][q] -> setScaleFunc(scale.first);
                }
            }
        }
    } else {
        int i = scale.second.split("-")[0].toInt();
        int j = scale.second.split("-")[1].toInt();
        _zoom[i][j] = scale.first;
    }
}

void MainWindow::factor_mid_level(QPair<double, QString> scale){
    int i = scale.second.split("-")[0].toInt();
    int j = scale.second.split("-")[1].toInt();
    double zoom_factor = _zoom_mid_level[i][j] + scale.first;
    if(_shared_zoom){
        for(int p=0; p < _levels.size(); p++){
            for(int q=0; q < _levels[p].size(); q++){
                if(_stage_status[p][q] == "true"){
                   _zoom_mid_level[p][q] = zoom_factor;
                }
            }
        }
    } else {
        _zoom_mid_level[i][j] = zoom_factor;
    }
    redraw_tree({0,0},true,false);
}

void MainWindow::on_checkBox_stateChanged(int arg1) //0 is off, 2 is on
{
    if(arg1 == 2){
        _shared_zoom = true;
    } else {
        _shared_zoom = false;
    }
}

void MainWindow::linked_pan(QPair<QPointF, QString> pan_val){
    bool update_left_overflow = false;
    bool change = false;
    bool update_right_overflow = false;
    bool change_right = false;
    if(_shared_zoom){
        for(int p=0; p < _levels.size(); p++){
            for(int q=0; q < _levels[p].size(); q++){
                if(_stage_status[p][q] == "true"){
                    update_left_overflow = false;
                    change = false;
                    update_right_overflow = false;
                    change_right = false;
                    if(_pan[p][q].rx() >= 1 && pan_val.first.rx() < 1){
                        update_left_overflow = true;
                    }
                    if(_pan[p][q].rx() < 1 && pan_val.first.rx() >= 1){
                        change = true;
                    }
                    //right overflow indicator
                    int max_size = _max_sizes_seqs[p][q]*(_block_size+_block_margin)*_zoom[p][q];
                    max_size = max_size - _width_open_parent_low;
                    if(max_size + _pan[p][q].rx() <= 1 && max_size + pan_val.first.rx() > 1){
                        update_right_overflow = true;
                    }
                    if(max_size + _pan[p][q].rx() > 1 && max_size + pan_val.first.rx() <= 1){
                        change_right = true;
                    }
                   _pan[p][q] = pan_val.first;
                   if(update_left_overflow){
                       _stage_rects[p][q] -> overflow_left(true);
                   } else if(change){
                       _stage_rects[p][q] -> overflow_left(false);
                   } else {
                        _parent_lows[p][q] -> setPosFunc(pan_val.first);
                   }
                   //right overflow
                   if(update_right_overflow){
                       _stage_rects[p][q] -> overflow_right(true);
                   } else if(change_right){
                       _stage_rects[p][q] -> overflow_right(false);
                   }
                }
            }
        }
    } else {
        int i = pan_val.second.split("-")[0].toInt();
        int j = pan_val.second.split("-")[1].toInt();
        int max_size = _max_sizes_seqs[i][j]*(_block_size+_block_margin)*_zoom[i][j];
        max_size = max_size - _width_open_parent_low;
        //left overflow indicator
        if(_pan[i][j].rx() >= 1 && pan_val.first.rx() < 1){
            update_left_overflow = true;
        }
        if(_pan[i][j].rx() < 1 && pan_val.first.rx() >= 1){
            change = true;
        }
        //right overflow indicator
        if(max_size + _pan[i][j].rx() <= 1 && max_size + pan_val.first.rx() > 1){
            update_right_overflow = true;
        }
        if(max_size + _pan[i][j].rx() > 1 && max_size + pan_val.first.rx() <= 1){
            change_right = true;
        }
        _pan[i][j] = pan_val.first;
        //left overflow
        if(update_left_overflow){
            _stage_rects[i][j] -> overflow_left(true);
        } else if(change){
            _stage_rects[i][j] -> overflow_left(false);
        }
        update_left_overflow = false;
        change = false;
        //right overflow
        if(update_right_overflow){
            _stage_rects[i][j] -> overflow_right(true);
        } else if(change_right){
            _stage_rects[i][j] -> overflow_right(false);
        }
        update_right_overflow = false;
        change_right = false;
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        tr("Open File"), "/", tr("CSV Files (*.csv)"));
    _fileName = fileName;

    //delete all
    _scene->clear();
    _scene_scatter->clear();
    _stages_data.clear();
    _levels.clear();
    _stage_tree.clear();
    _freq_per_stage.clear();
    _stage_rects.clear();
    _stage_rects_old.clear();
    _stage_status.clear();
    _stage_high.clear();
    _scatter_points.clear();
    _sel_dots.clear();
    _dots_high.clear();
    _dots_scatter_list.clear();
    _start = true;
    _seq_data_stages.clear();
    _lowest_ind_stage.clear();
    _cluster_info.clear();
    _dwarf_willows.clear();
    _axis_val.clear();
    _colors_dots.clear();
    _total_freqs.clear();
    _patterns.clear();
    _pat_id_to_pat.clear();
    _unique_pat.clear();
    _most_freq_pat.clear();
    _total_pats.clear();
    _max_val = 0;
}


void MainWindow::on_pushButton_3_clicked()
{
    if(!_run_time_test){
        QString seqFileName = QFileDialog::getOpenFileName(this,
            tr("Open File"), "/", tr("CSV Files (*.csv)"));
        _seqFileName = seqFileName;
    }

    //run
    qDebug() << "one";
    _data = load_data(_fileName);
    get_seq_info();
    _num_seqs = _data->item(_data->rowCount()-1,0)->text().toInt() +1;
    _cats = _total_count_event.keys();
    QList<double> tmp = _total_count_event.values();
    std::sort(tmp.begin(), tmp.end());
    for(int i=tmp.size()-1; i >= 0;i--){

        if(tmp.size()-1-i < _colors.size()){
            _c[_total_count_event.key(tmp[i])] = _colors[_colors.size()-1-(tmp.size() - 1 - i)];
        } else {
            _c[_total_count_event.key(tmp[i])] = QColor(217,217,217, 175);
        }
    }
    //colors from https://colorbrewer2.org/
    if(_fileName.split("all_buildings").size() > 1 || _fileName.split("final").size() > 1){
        _c["q1"] = QColor(0, 68, 27, 175);
        _c["q2"] = QColor(0, 109, 44,175);
        _c["q3"] = QColor(35, 139, 69, 175);
        _c["q4"] = QColor(65, 171, 93, 175);
        _c["q5"] = QColor(161, 217, 155, 175);
        _c["q6"] = QColor(199, 233, 192, 175);
        _c["-"] = QColor(106,61,154, 175);
        _c["r6"] = QColor(217,217,217, 175);
        _c["r5"] = QColor(217,217,217, 175);
        _c["r4"] = QColor(217,217,217, 175);
        _c["r3"] = QColor(217,217,217, 175);
        _c["r2"] = QColor(217,217,217,175);
        _c["r1"] = QColor(217,217,217, 175);
        _c["t6"] = QColor(217,217,217, 175);
        _c["t5"] = QColor(217,217,217, 175);
        _c["t4"] = QColor(217,217,217, 175);
        _c["t3"] = QColor(217,217,217, 175);
        _c["t2"] = QColor(217,217,217,175);
        _c["t1"] = QColor(217,217,217, 175);
        _c["s6"] = QColor(217,217,217, 175);
        _c["s5"] = QColor(217,217,217, 175);
        _c["s4"] = QColor(217,217,217, 175);
        _c["s3"] = QColor(217,217,217, 175);
        _c["s2"] = QColor(217,217,217,175);
        _c["s1"] = QColor(217,217,217, 175);
    } else if(_fileName.split("test").size() > 1){        
        _c["-"] = QColor(106,61,154, 175);
        _c["B"] = QColor(128,177,211,175);
        _c["O"] = QColor(177,89,40, 175);
        _c["G"] = QColor(51,160,44, 175);
        _c["R"] = QColor(227,26,28, 175);
    }

    for(int i=0; i< _cats.size(); i++){
        QListWidgetItem* item = new QListWidgetItem(tr(_cats[i].toStdString().c_str()), ui->listWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        item->setBackground(_c[_cats[i]]);
        _cats_filter.append(true);
    }
    for(int i=0; i<_seq_info.values().size(); i++){
        QListWidgetItem* item = new QListWidgetItem(tr(_seq_info[i].toStdString().c_str()), ui->listWidget_2);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        _seqs_filter.append(true);
    }
    if(_mode_staging_global == "patterns"){
        //pattern file should be sorted on start index
        QSet<QString> set_pat;
        QList<QString> tmp_path = QDir::currentPath().split("/");
        QString path = "";
        for(int i=0; i<tmp_path.size()-1; i++){
            path += tmp_path[i] + "/";
        }
        if(!_run_time_test){
            _pattern_path = path + "pattern_file.csv";
        }
        QFile file(_pattern_path);
        QString Alldata;
        QStringList rowAsString;
        QStringList row;

        if (file.open(QFile::ReadOnly)){
               Alldata = file.readAll();
               rowAsString = Alldata.split("\n");
               file.close();
        }
        for (int i = 1; i < rowAsString.size(); i++){
            row = rowAsString.at(i).split(",");
            QList<QString> pat;
            if(row.size() > 1){
               pat.append(row[0]);
               pat.append(row[row.size()-3]);
               pat.append(row[row.size()-2]);
               pat.append(row[row.size()-1]);
               _patterns.append(pat);
               set_pat.insert(row[0]);
               if(!_pat_id_to_pat.contains(row[0])){
                   QList<QString> pat;
                   pat.append(row[1].split("[")[1].replace("'", ""));
                   for(int q = 2; q<row.size()-5; q++){
                       QString tmp = row[q].replace("'", "");
                       pat.append(tmp.replace(" ", ""));
                   }
                   QString last = row[row.size()-5].split("]")[0];
                   last = last.replace("'", "");
                   last = last.replace(" ", "");
                   pat.append(last);
                   _pat_id_to_pat[row[0]] = pat;
               }
               if(!_total_pats.contains(row[0])){
                   _total_pats[row[0]] = 1;
               } else {
                   _total_pats[row[0]] ++;
               }
            }
        }
        for (auto i = set_pat.begin(), end = set_pat.end(); i != end; ++i){
            _unique_pat.append(*i);
        }
        QList<QPair<int,QString>> most_freq_pat_list;
        for (auto i = _total_pats.begin(), end = _total_pats.end(); i != end; ++i){
            most_freq_pat_list.append(qMakePair(i.value(), i.key()));
        }
        std::sort(std::begin(most_freq_pat_list), std::end(most_freq_pat_list));
        for(int i=most_freq_pat_list.size()-1; i >= 0; i--){
            if(i>most_freq_pat_list.size()-1-20){
                _most_freq_pat.append(most_freq_pat_list[i].second);
            }
        }

    }
    qDebug() << "two";
    QElapsedTimer timer;
    ui->progressBar->setVisible(true);
    ui->progressBar->setValue(0);
    QApplication::processEvents();
    timer.start();
    std::vector<std::string> cats_vec;
    std::vector<bool> cats_filter_vec;
    for(int i =0; i< _cats.size();i++){
       cats_vec.push_back(_cats[i].toStdString());
    }
    for(int i =0; i< _cats_filter.size();i++){
       cats_filter_vec.push_back(_cats_filter[i]);
    }
    std::unordered_map<std::string, double> total_counts_event_map;
    for (auto k = _total_count_event.cbegin(), end = _total_count_event.cend(); k != end; ++k){
        total_counts_event_map.insert({k.key().toStdString(), k.value()});
    }
    _stages_data = staging(_data, _num_seqs, _min_events_stage, _max_stages, _max_dev,
                           _mode_staging_global, 0, total_counts_event_map, ui->progressBar,
                           cats_vec, cats_filter_vec, _mode_staging_local, _run_time_test,
                           _pattern_path, _staging_eps);
    qDebug() << timer.elapsed();
    qDebug() << "total staging";
    ui->progressBar->setValue(100);
    ui->progressBar->setVisible(false);
    if(_stages_data.size() > 0){
        if(_stages_data[0].size() > 0){
            _num_stages = _stages_data[0].size() + 1;            
            qDebug() << "three";
            drawStages();
            qDebug() << "four";
            cluster_seqs_stages();
            stage_tree();
            qDebug() << "five";
            draw_tree();
            qDebug() << "six";
            QElapsedTimer timer3;
            timer3.start();
            draw_scatter();
            qDebug() << timer3.elapsed();
            qDebug() << "seven";
        } else{
            QMessageBox msgBox;
            msgBox.setText("No stages discovered.");
            msgBox.exec();
        }
    } else{
        QMessageBox msgBox;
        msgBox.setText("No stages discovered.");
        msgBox.exec();
    }
}


void MainWindow::on_norm_stateChanged(int arg1)
{
    if(arg1 == 2){
        _normalized_bar_charts = true;
    } else {
        _normalized_bar_charts = false;
    }
    if(_mode == "high"){
        redraw_tree({0,0},true,false);
    }
}


void MainWindow::on_spinBox_4_valueChanged(int arg1)
{
    _cluster_on = arg1;
    if(_cluster){
//        Are in cluster mode already and want to change clustering based on clustering in cluster x
        int row_margin = 2;
        QParallelAnimationGroup* anim_dwarf_willows = new QParallelAnimationGroup;
        for(int i=0;i< _levels.size();i++){
            for(int j=0; j< _levels[i].size(); j++){
                if(_stage_status[i][j] == "true"){
                    for(int k=0;k<_dwarf_willows[i][j].size(); k++){ //list with all blocks per seq [[blocks seq 0],[block seq 1]]
                        int new_y = 0;
                        QList<int> key;
                        std::map<int, QList<int>> stage_clusters =
                                _cluster_info[_cluster_on];
                        for(std::map<int,QList<int>>::iterator it = stage_clusters.begin(); it != stage_clusters.end(); ++it) {
                          key.append(it->first);
                        }
                        bool done = false;
                        for(int s=0; s< key.size();s++){
                            for(int u =0; u<stage_clusters[key[s]].size(); u++){
                                if(stage_clusters[key[s]][u] == k){
                                    done = true;
                                    break;
                                }
                                new_y += _block_size + row_margin;
                            }
                           if(done){
                               break;
                           }
                           new_y += 10;
                        }
                        for (int l=0;l<_dwarf_willows[i][j][k].size(); l++){
                            QPropertyAnimation* anim = new QPropertyAnimation(_dwarf_willows[i][j][k][l] , "pos");
                            anim->setStartValue(_dwarf_willows[i][j][k][l]->pos());
                            anim->setEndValue(QPointF(_dwarf_willows[i][j][k][l]->pos().x(), new_y));
                            anim->setDuration(1000);
                            anim->setEasingCurve(QEasingCurve::OutCurve);
                            anim_dwarf_willows->addAnimation(anim);

                        }
                    }
                 }
              }
        }
        connect(anim_dwarf_willows,SIGNAL(finished()), this,SLOT(finished_anim_clus()));
        anim_dwarf_willows->start();
    }
}

void MainWindow::on_eps_cluster_tree_valueChanged(double arg1)
{
    _eps_clusters_tree = arg1;
    qDebug() << "eps" << _eps_clusters_tree;
    _cluster_info.clear();
    cluster_seqs_stages();
    redraw_tree({0,0}, true, false);
}

//running times tests
void MainWindow::on_pushButton_4_clicked()
{
//    QString str_case = "type_1000";
//    bool events = false;

//    //length
//    if(str_case == "length_100"){
//        _fileName = "";
//        _seqFileName = "";
//        _pattern_path = "";
//        _max_stages = 0.025;
//        _staging_eps = 0.2;
//    } else if(str_case == "length_75"){
//        _fileName = "";
//        _seqFileName = "";
//        _pattern_path = "";
//        _max_stages = 0.025;
//        _staging_eps = 0.2;
//    } else if(str_case == "length_50"){
//        _fileName = "";
//        _seqFileName = "";
//        _pattern_path = "";
//        _max_stages = 0.025;
//        _staging_eps = 0.2;
//    }else if(str_case == "length_25"){
//        _fileName = "";
//        _seqFileName = "";
//        _pattern_path = "";
//        _max_stages = 0.025;
//        _staging_eps = 0.2;
//    } else if(str_case == "test"){
//        _fileName = "";
//        _seqFileName = "";
//        _pattern_path = "";
//        _max_stages = 0.15;
//        _staging_eps = 0.2;
//    }

//    //number of sequences
//    QString base_seqs = "";
//    if(str_case == "seqs_10"){
//        _fileName = base_seqs + "";
//        _seqFileName = base_seqs + "";
//        _pattern_path = base_seqs + "";
//        _max_stages = 0.025;
//        _staging_eps = 0.2;
//    } else if(str_case == "seqs_50"){
//        _fileName = base_seqs + "";
//        _seqFileName = base_seqs + "";
//        _pattern_path = base_seqs + "";
//        _max_stages = 0.025;
//        _staging_eps = 0.2;
//    } else if(str_case == "seqs_100"){
//        _fileName = base_seqs + "";
//        _seqFileName = base_seqs + "";
//        _pattern_path = base_seqs + "";
//        _max_stages = 0.025;
//        _staging_eps = 0.2;
//    }else if(str_case == "seqs_500"){
//        _fileName = base_seqs + "";
//        _seqFileName = base_seqs + "";
//        _pattern_path = base_seqs + "";
//        _max_stages = 0.025;
//        if(events){
//            _staging_eps = 0.2;
//        } else{
//             _staging_eps = 0.5;
//        }
//    } else if(str_case == "seqs_1000"){
//        _fileName = base_seqs + "";
//        _seqFileName = base_seqs + "";
//        _pattern_path = base_seqs + "";
//        _max_stages = 0.025;
//        if(events){
//            _staging_eps = 0.2;
//        } else{
//             _staging_eps = 0.5;
//            }
//    }


//    //number of event types
//    QString base_type = "";
//    if(str_case == "type_10"){
//        _fileName = base_type + "";
//        _seqFileName = "";
//        _pattern_path = base_type + "";
//        _max_stages = 0.05;
//        _staging_eps = 0.2;
//    } else if(str_case == "type_50"){
//        _fileName = base_type + "";
//        _seqFileName = "";
//        _pattern_path = base_type + "";
//        _max_stages = 0.05;
//        _staging_eps = 0.5;
//    } else if(str_case == "type_100"){
//        _fileName = base_type + "";
//        _seqFileName = "";
//        _pattern_path = base_type + "";
//        _max_stages = 0.05;
//        _staging_eps = 0.5;
//    }else if(str_case == "type_500"){
//        _fileName = base_type + "";
//        _seqFileName = "";
//        _pattern_path = base_type + "";
//        _max_stages = 0.1;
//        if(events){
//            _staging_eps = 0.7;
//        } else{
//             _staging_eps = 0.5;
//        }
//    } else if(str_case == "type_1000"){
//        _fileName = base_type + "";
//        _seqFileName = "";
//        _pattern_path = base_type + "";
//        _max_stages = 0.1;
//        if(events){
//            _staging_eps = 0.7;
//        } else{
//             _staging_eps = 0.2;
//            }
//    }

//    if(events){
//        _mode_staging_global = "entropy";
//        _mode_staging_local = "entropy";
//    } else {
//        _mode_staging_global = "patterns";
//        _mode_staging_local = "patterns";
//    }
//    _run_time_test = true;
//    _max_dev = 100;
//    _min_events_stage = 500;

//    qDebug() << str_case << events;
//    //run
//    on_pushButton_3_clicked();
//    //rerun
//    on_pushButton_clicked();
}

