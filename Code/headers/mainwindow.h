#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGraphicsScene>
#include <QTableWidget>
#include <QVector>
#include "child_low.h"
#include "parent_low.h"
#include "qlistwidget.h"
#include "tree.h"
#include "dots.h"
#include <QPropertyAnimation>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void init();

private slots:
    void stage_rect_clicked(QVector<double> index);
    void dot_clicked(int index);
    void dot_hover_true(int index);
    void dot_hover_false(int index);
    void box_selection(bool selection);
    void finished_animation();
    void finished_anim_clus();
    void on_low_level_stateChanged(int arg1);
    void on_high_level_stateChanged(int arg1);
    void on_cluster_button_stateChanged(int arg1);
    void rect_bar_clicked(QString rect);
    void rect_event_clicked(QString rect);
    void seq_high_cluster(int ind);
    void seq_low_cluster(int ind);
    void linked_zoom(QPair<double, QString> scale);
    void factor_mid_level(QPair<double, QString> scale);
    void linked_pan(QPair<QPointF, QString> pan_val);

    void on_time_stateChanged(int arg1);

    void on_similarity_stateChanged(int arg1);

//    void on_spinBox_valueChanged(int arg1);

    void on_spinBox_2_valueChanged(int arg1);

    void on_spinBox_3_valueChanged(int arg1);

    void on_checkBox_2_stateChanged(int arg1);

    void on_checkBox_3_stateChanged(int arg1);

    void on_checkBox_4_stateChanged(int arg1);

    void on_checkBox_5_stateChanged(int arg1);

    void on_pushButton_clicked();

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_listWidget_2_itemClicked(QListWidgetItem *item);

    void on_checkBox_stateChanged(int arg1);

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_norm_stateChanged(int arg1);

    void on_spinBox_4_valueChanged(int arg1);

    void on_doubleSpinBox_valueChanged(double arg1);


    void on_mid_level_stateChanged(int arg1);

    void on_eps_cluster_tree_valueChanged(double arg1);

    void on_pushButton_4_clicked();

private:
    Ui::MainWindow *ui;
    QGraphicsScene* _scene;
    QGraphicsScene* _scene_scatter;
    QTableWidget* _data;
    QTableWidget* _data_orignal;
    QVector<QVector<QVector<double>>> _stages_data;
    QVector<QVector<QString>> _levels;
    QList<QString> _stage_tree;
    int _num_stages;
    int _num_seqs;
    QVector<QMap<QString, double>> _freq_per_stage;
    QVector<QMap<QString, double>> _pat_per_stage;
    int _max_val = 0;
    int _max_val_leaves = 0;
    int _width, _height;
    QString _mode = "high";
    QVector<QVector<tree*>> _stage_rects;
    QVector<QVector<tree*>> _stage_rects_old;
    QVector<QVector<QString>> _stage_status; //true is shown false is not
    QVector<QVector<QString>> _stage_high; //true is high false is not
    QList<QPointF> _scatter_points;
    QList<int> _sel_dots; //selected through box selection
    QList<bool> _dots_high;
    QList<dots*> _dots_scatter_list;
    bool _start = true;
    bool _cluster = false;
    QList<QList<QList<QPair<QString, int>>>> _seq_data_stages;
    QList<int> _lowest_ind_stage;
    QList<std::map<int, QList<int>>> _cluster_info;
    QVector<QVector<QVector<QVector<child_low*>>>> _dwarf_willows;
    bool _color_scat_time = true;
    QVector<double> _axis_val;
    QList<QColor> _colors_dots;
    QMap<QString, int> _total_freqs;
    int _min_events_stage = 100;
    double _max_stages = 0.05;
    int _max_dev = 50;
    bool _normalized_bar_charts = false;
    QString _mode_staging_global = "entropy";
    QString _mode_staging_local = "entropy";
    double _eps_clusters_tree = 0.2;
    QMap<QString, double> _total_count_event;
    qreal _width_closed_rect = 5;
    qreal _height_closed_rect = 10;
    QMap<int, QPair<QString, QString>> _start_end_time_stage;
    int _len_seq;
    QMap<int, QString> _seq_info;
    QList<QString> _cats;
    QList<bool> _cats_filter;
    QList<bool> _seqs_filter;
    //colors from https://colorbrewer2.org/
    QList<QColor> _colors = {QColor(255,255,179, 175), QColor(106,61,154, 175), QColor(31,120,180, 175),
        QColor(51,160,44, 175), QColor(255,127,0,175),
        QColor(251,154,153, 175), QColor(227,26,28, 175), QColor(166,206,227, 175),
        QColor(253,191,111, 175), QColor(177,89,40,175),
        QColor(255,237,111, 175),
        QColor(128,177,211, 175), QColor(179,222,105, 175),
        QColor(204,235,197, 175), QColor(188,128,189,175)};
    QMap<QString, QColor> _c;
    QList<QString> _high_through_click;
    QPointF _prev_mouse;
    bool _pat_created = false;
    int _block_size = 5;
    int _block_margin = 1;
    int _hover_cluster_ind = -1;
    QList<QString> _filtered_out_cats;
    QList<QList<parent_low*>> _parent_lows;
    QList<QList<double>> _zoom;
    QList<QList<double>> _zoom_mid_level;
    QList<QList<QPointF>> _pan;
    bool _shared_zoom = false;
    QString _fileName;
    QString _seqFileName;
    QList<QList<double>> _max_sizes_seqs;
    double _width_open_parent_low;
    QList<QList<QString>> _patterns;
    QList<QString> _unique_pat;
    QList<int> _num_pat_stage;
    QMap<QString, QList<QString>> _pat_id_to_pat;
    QMap<QString, int> _total_pats;
    QList<QString> _most_freq_pat;
    QList<QList<int>> _high_patterns_events;
    QVector<QVector<QMap<QString, double>>> _pat_per_stage_seq;
    int _cluster_on = -1;
    bool _run_time_test = false;
    QString _pattern_path = "";
    double _staging_eps = 0.2;

    QTableWidget* load_data(QString file);
    void drawStages();
    void load_stages();
    void stage_tree();
    void make_hier_tree(int current_split, QString text_parent, int count, int level, QVector<QVector<double>> data, int max_height);
    void draw_tree();
    void redraw_tree(QVector<double> index, bool hover, bool animate);
    qreal calc_width_stages(QList<QString> names, QList<int> names_numbers, QString stage);
    void draw_scatter();
    void redraw_scatter();
    void cluster_seqs_stages();
    void get_seq_info();

};
#endif // MAINWINDOW_H
