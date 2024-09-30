#ifndef STAGING_H
#define STAGING_H

#include "qelapsedtimer.h"
#include "qprogressbar.h"
#include "qtablewidget.h"
#include "dbscan.h"
#include <math.h>
#include <QFile>
#include <QDir>

std::vector<std::vector<double>> stages;
//parameters for running time tests
//int total_split_runs = 0;
//double total_time_split_global = 0;
//double total_time_split_local = 0;

double entropy(std::unordered_map<std::string, double> &counts, int length){
    if(length == 0) {
        return 0;
    } else {
        std::vector<std::string> keys;
        std::vector<double> prob;
        double entropy = 0;
        for(std::unordered_map<std::string,double>::iterator it = counts.begin(); it != counts.end(); ++it) {
          keys.push_back(it->first);
        }
        for(int i=0;i<keys.size(); i++){
            prob.push_back((1.0*counts[keys[i]]) / length);
        }
        for(int i=0;i<prob.size(); i++){
            if(prob[i] == 0){
                entropy += 0;
            } else {
                entropy += -1*prob[i]*log2(prob[i]);
            }
        }
        return entropy;
    }
}

double info_gain(std::unordered_map<std::string, double> &left, std::unordered_map<std::string, double> &right, std::unordered_map<std::string, double> &total,
                 std::vector<std::string> &cats, std::vector<bool> &cats_filter){

    for(int i=0; i< cats.size(); i++){
        if(!cats_filter[i]){
            left.erase(cats[i]);
            right.erase(cats[i]);
            total.erase(cats[i]);
        }
    }

    double length_left = 0, length_right =0, length_total = 0;
    for(auto it = left.begin(); it != left.end(); ++it) {
      length_left += it->second;
    }
    for(auto it = right.begin(); it != right.end(); ++it) {
      length_right += it->second;
    }    
    for(auto it = total.begin(); it != total.end(); ++it) {
      length_total += it->second;
    }

    double ig = entropy(total, length_total) - ((length_left/length_total) *
                        entropy(left, length_left) + (length_right/length_total) *
                                                entropy(right, length_right));
    return ig;
}

double dist_two_seq(std::unordered_map<std::string, double> &s1, std::unordered_map<std::string, double> &s2, std::vector<std::string> &cats, std::vector<bool> &cats_filter){

    for(int i=0;i<cats.size();i++){
        if(!cats_filter[i]){
            s1.erase(cats[i]);
            s2.erase(cats[i]);
        }
    }
    std::unordered_map<std::string, std::vector<double>> map_freqs;
    int sum_s1 = 0;
    int sum_s2 = 0;
    for(auto i = s1.cbegin(), end = s1.cend(); i != end; ++i){
        if(map_freqs.count(i->first) > 0){
            map_freqs[i->first][0] = i->second;
        } else {
            map_freqs[i->first] = {i->second,0};
        }
        sum_s1 += i->second;
    }
    for(auto i = s2.cbegin(), end = s2.cend(); i != end; ++i){
        if(map_freqs.count(i->first) > 0){
            map_freqs[i->first][1] = i->second;
        } else {
            map_freqs[i->first] = {0, i->second};
        }
        sum_s2 += i->second;
    }
    //based on jensen shannon distance from https://docs.scipy.org/doc/scipy/reference/generated/scipy.spatial.distance.jensenshannon.html
    std::vector<double> p_s1;
    std::vector<double> p_s2;
    std::vector<double> p;
    for (auto i = map_freqs.cbegin(), end = map_freqs.cend(); i != end; ++i){
        p_s1.push_back(i->second[0]*1.0 / sum_s1*1.0);
        p_s2.push_back(i->second[1]*1.0 / sum_s2*1.0);
        p.push_back(((i->second[0]*1.0 / sum_s1*1.0) + (i->second[1]*1.0 / sum_s2*1.0)) / 2.0);
    }

    double left_sum = 0;
    double right_sum = 0;
    for(int i =0; i < p.size(); i++){
        if(p_s1[i] > 0 && p[i]>0){
            left_sum += p_s1[i]*log((p_s1[i]/p[i]));
        } else if(p_s1[i] < 0 && p[i]<0) {
            left_sum += std::numeric_limits<float>::infinity();
            qDebug() << p << p_s1 << p_s2 << "impossible";
        }

        if(p_s2[i] > 0 && p[i]>0){
            right_sum += p_s2[i]*log((p_s2[i]/p[i]));
        } else if(p_s2[i] < 0 && p[i]<0) {
            right_sum += std::numeric_limits<float>::infinity();
            qDebug() << p << p_s1 << p_s2 << "impossible";
        }
    }
    double total = sqrt(((left_sum+right_sum) / 2.0));
    return total;
}

std::vector<std::unordered_map<std::string, double>> split(std::vector<std::vector<std::string>> &data, int min_events_stage, int level,
                    int offset, bool cluster, std::vector<int> clusters, std::unordered_map<std::string, double> &total_counts,
                    std::vector<std::string> &cats, std::vector<bool> &cats_filter, QString mode, int len_seq, double max_stages,
                                   double ig_cum, int len_seq_total){
//    total_split_runs ++;
    if(min_events_stage == 0){
        min_events_stage = 1;
    }
    std::vector<int> candidates;
    std::vector<std::string> prev = data[0];
    bool cand = false;
    std::vector<double> ig = {-100,-1,-1,-1};
    std::vector<std::vector<std::unordered_map<std::string, double>>> counts;
    std::unordered_map<std::string, double> total_left;
    std::unordered_map<std::string, double> total_right;

    //get total counts and all candidate ind
    //for each candidate split get info gain
    std::unordered_map<std::string, double> left;
    std::unordered_map<std::string, double> right = total_counts;
    std::vector<double> info_gain_cand;
    std::vector<std::vector<std::string>> poss_on_cut;

    double ratio = 0; //how much of event is the same of two col
    int previous = NULL;
    for(int i=0; i<data.size(); i++){
        if(mode == "patterns"){
            if(cluster){                
               if(count(clusters.begin(), clusters.end(), stoi(data[i][1])) == 0){            
                   continue;
               }
            }
            //compute patterns on left side            
            if(stoi(data[i][2]) - offset >= min_events_stage &&
                    stoi(data[i][2]) - offset <= len_seq - min_events_stage
                    && i > 0){
                if(stoi(data[i][2]) != previous){

// for detailed cuts
                    std::vector<int> remove_lst;
                    if(poss_on_cut.size() > 0 && len_seq_total < 5000){
                        for(int j=0; j < poss_on_cut.size(); j++){
                            if(stoi(data[i][2]) >= stoi(poss_on_cut[j][3])){
                                remove_lst.push_back(j);
                                if(left.count(poss_on_cut[j][0]) > 0){
                                    left[poss_on_cut[j][0]] ++;
                                } else {
                                    left[poss_on_cut[j][0]] += 1;
                                }
                                total_counts[poss_on_cut[j][0]] ++;
                            }
                        }
                        //for all in remove, remove
                        for(int j=remove_lst.size()-1; j>=0 ; j--){
                           poss_on_cut.erase(poss_on_cut.begin() + remove_lst[j]);
                        }
                    }
// until here

                    //get pattern info gain
                    //add i to candidates
                    //dist entropy and info gain                    
                    double info_pat = info_gain(left, right, total_counts, cats, cats_filter);
                    info_gain_cand.push_back(info_pat);
                    candidates.push_back(i);
                    counts.push_back({left, right});
                 }
            }

//comment in for detailed cuts
            if(len_seq_total < 5000){
                if(i+1<data.size()){
                    if(stoi(data[i+1][2]) >= stoi(data[i][3])){
                        if(left.count(data[i][0]) > 0){
                            left[data[i][0]] ++;
                        } else {
                            left[data[i][0]] += 1;
                        }
                        right[data[i][0]] --;
                    }
                    else {
                        //patterns on cut
                        poss_on_cut.push_back(data[i]);
                        right[data[i][0]] --;
                        total_counts[data[i][0]] --;
                    }
                }
// until here
            } else {
                if(left.count(data[i][0]) > 0){
                    left[data[i][0]] ++;
                } else {
                    left[data[i][0]] += 1;
                }
                right[data[i][0]] --;
               }
            previous = stoi(data[i][2]);
        } else {
            cand = true;
            ratio = 0;
            for(int j=0; j<data[0].size(); j++){
                if(cluster){
                   if(count(clusters.begin(), clusters.end(), j) == 0){
                       continue;
                   }
                }
                if( prev[j] == data[i][j]){
                    if(cluster){
                       ratio += 1.0/clusters.size()*1.0;
                    } else {
                        ratio += 1.0/prev.size()*1.0;
                    }
                }
                if(left.count(data[i][j]) > 0){
                    left[data[i][j]] ++;
                } else {
                    left[data[i][j]] += 1;
                }
                right[data[i][j]] --;
            }
            if( i < min_events_stage || i > data.size()-min_events_stage){
                cand = false;
            } else if(ratio > 0.75){
                cand = false;
            }
            if(cand && i >0){
//                //dist entropy and info gain
                info_gain_cand.push_back(info_gain(counts[counts.size()-1][0],counts[counts.size()-1][1], total_counts, cats, cats_filter));
                candidates.push_back(i);
            }
            prev = data[i];
            counts.push_back({left, right});
        }
    }
    ig[2] = level;
    int index;
    for(int i =0; i< info_gain_cand.size(); i++){
        if(info_gain_cand[i] > ig[1]){
            ig[1] = info_gain_cand[i];
            if(mode == "patterns"){
                ig[0] = stoi(data[candidates[i]][2]);
                index = i;
                ig[3] = candidates[i];
            } else {
                ig[0] = candidates[i]+offset;
                index = candidates[i]-1;
            }
        }
    }
    if((ig[1] / ig_cum) < max_stages){ //threshold
        ig[0] = -100;
    }
    if(ig[0] > -1){
        total_left = counts[index][0];
        total_right = counts[index][1];
    }
    std::unordered_map<std::string, double> ig_map;
    ig_map["0"] = ig[0]; //index
    ig_map["1"] = ig[1]; //ig score
    ig_map["2"] = ig[2];
    ig_map["3"] = ig[3];
    return {ig_map, total_left, total_right};
}

void get_stages_global(std::vector<std::vector<std::string>> data, int min_events_stage, int level,
                       double max_stages, QString mode, int offset, std::unordered_map<std::string, double> &total_count_event,
                       std::vector<std::string> &cats, std::vector<bool> &cats_filter, int len_seq, double ig_cum, int len_seq_total){
//    QElapsedTimer timer10;
//    timer10.start();
    std::vector<std::unordered_map<std::string, double>> stage = split(data, min_events_stage, level, offset, false, {}, total_count_event,
                                               cats, cats_filter, mode, len_seq, max_stages, ig_cum, len_seq_total);
//    total_time_split_global += timer10.elapsed();
    if(stage[0]["0"] != -100){
        stages.push_back({stage[0]["0"], stage[0]["1"], stage[0]["2"]});
        ig_cum += stage[0]["1"];
        int ind = stage[0]["0"] - offset;
        if(mode == "patterns"){
            get_stages_global(std::vector<std::vector<std::string>>(data.begin(), data.begin() + stage[0]["3"]), min_events_stage, level+1, max_stages, mode, offset, stage[1], cats, cats_filter, ind, ig_cum, len_seq_total);
            get_stages_global(std::vector<std::vector<std::string>>(data.begin() + stage[0]["3"], data.end()), min_events_stage, level+1, max_stages, mode, ind + offset, stage[2], cats, cats_filter, len_seq-ind, ig_cum, len_seq_total);

        } else {
            get_stages_global(std::vector<std::vector<std::string>>(data.begin(), data.begin() + ind), min_events_stage, level+1, max_stages, mode, offset, stage[1], cats, cats_filter, ind, ig_cum, len_seq_total);
            get_stages_global(std::vector<std::vector<std::string>>(data.begin() + ind, data.end()), min_events_stage, level+1, max_stages, mode, ind + offset, stage[2], cats, cats_filter, len_seq-ind, ig_cum, len_seq_total);

        }

    }
}


std::vector<std::vector<std::vector<double>>> local_staging(std::vector<std::vector<std::string>> &data, QString mode, int min_events_stage,
                   int max_dev, std::vector<std::vector<std::vector<double>>> &local_stages, std::vector<std::string> &cats,
                   std::vector<bool> &cats_filter, QString mode_local, std::vector<std::vector<std::string>> &patterns,
                   double max_stages, int len_seq_total, double staging_eps){

    int index_pat_file = 0;
    for(int i=0; i< stages.size(); i++){        
        //get two consec stages
        std::vector<std::vector<std::string>> stage1and2;
        int start_ind = 0;
        if(i ==0){
            if(stages.size() > 1){
               stage1and2 = std::vector<std::vector<std::string>>(data.begin(), data.begin() + stages[i+1][0]);
            } else {
                stage1and2 = data;
            }
        } else if(i == stages.size()-1){
            if(stages.size() > 1){
               stage1and2 = std::vector<std::vector<std::string>>(data.begin() + stages[i-1][0], data.end());
               start_ind = stages[i-1][0];
            } else {
                stage1and2 = data;
            }
        } else {   
            stage1and2 = std::vector<std::vector<std::string>>(data.begin() + stages[i-1][0], data.begin() + stages[i+1][0]);
            start_ind = stages[i-1][0];
        }
        //get patterns in two consec stages
        std::vector<std::vector<std::string>> patterns_stage1and2;
        std::vector<std::unordered_map<std::string, double>> total_counts_pat_per_seq;
        total_counts_pat_per_seq.resize(stage1and2[0].size());
        if(mode_local == "patterns"){
            int target;
            int min = 0;
            if(i == stages.size()-1){
                target = data.size();
                if(stages.size() > 1){
                    min = stages[i-1][0];
                }
            } else {
                target = stages[i+1][0];
                if(i > 0){
                   min = stages[i-1][0];
                }
            }
            bool index_saved = false;
            for(int j=index_pat_file; j<patterns.size(); j++){
                if(stoi(patterns[j][2]) >= min && stoi(patterns[j][3]) <= target){
                    patterns_stage1and2.push_back(patterns[j]);
                    if(total_counts_pat_per_seq[stoi(patterns[j][1])].count(patterns[j][0])){
                        total_counts_pat_per_seq[stoi(patterns[j][1])][patterns[j][0]] ++;

                    } else{
                        total_counts_pat_per_seq[stoi(patterns[j][1])][patterns[j][0]] = 1;
                    }
                }
                if(stoi(patterns[j][2]) >= stages[i][0] && !index_saved){
                    index_pat_file = j;
                    index_saved = true;
                }
                if(stoi(patterns[j][2]) >= target){
                    break;
                }
            }

        }

        //get all seqs in conseq stages
        std::vector<std::unordered_map<std::string, double>> total_counts_per_seq;
        total_counts_per_seq.resize(stage1and2[0].size());
        std::vector<std::vector<std::string>> seqs;
        seqs.resize(stage1and2[0].size());
        for(int j = 0; j < stage1and2[0].size(); j++){
            for(int k=0; k<stage1and2.size(); k++){
                seqs[j].push_back(stage1and2[k][j]);
                //get dict with freq for each stage
                if(total_counts_per_seq[j].count(stage1and2[k][j])){
                    total_counts_per_seq[j][stage1and2[k][j]] ++;
                } else{
                    total_counts_per_seq[j][stage1and2[k][j]] = 1;
                }
            }
        }

        //get dist metric of seqs
        QList<QList<double>> dist_metr;
        std::vector<std::string> empty;
        std::vector<bool> empty2;
        dist_metr.resize(seqs.size());
        double length_seq = seqs[0].size();

        for(int j=0;j<seqs.size(); j++){
            for(int k=0;k<seqs.size(); k++){
                if(k <= j){
                    if (k == j){
                        dist_metr[j].append(0);
                    } else {                        
                        double dist;
                        if(mode_local == "patterns"){
                            dist = dist_two_seq(total_counts_pat_per_seq[j],total_counts_pat_per_seq[k],empty,empty2);
//                          if one histogram of the pattern occurences of a subsequence is empty and the other is not the dist is 0
//                            it is more logical is this becomes 1
                            if(dist == 0){
                                int total_j = 0;
                                int total_k = 0;
                                for (auto s = total_counts_pat_per_seq[j].cbegin(), end = total_counts_pat_per_seq[j].cend(); s != end; ++s){
                                    total_j += s -> second;
                                }
                                for (auto s = total_counts_pat_per_seq[k].cbegin(), end = total_counts_pat_per_seq[k].cend(); s != end; ++s){
                                    total_k += s -> second;
                                }
                                if((total_j != 0 && total_k == 0) || (total_k != 0 && total_j == 0)){
                                    dist = 1;
                                }
                            }
                        } else {
                            dist = dist_two_seq(total_counts_per_seq[j],total_counts_per_seq[k], cats,cats_filter);                            
                        }
                        dist_metr[j].append(dist);
                        dist_metr[k].append(dist);
                    }
                }
            }
        }

        //get clusters        
        auto dbscan = DBSCAN<std::vector<std::string>, float>();
        dbscan.Run(&seqs, seqs[0].size(), staging_eps, 2, dist_metr); //old value 0.2f
        auto noise = dbscan.Noise;
        auto clusters = dbscan.Clusters;
        std::vector<std::unordered_map<std::string, double>> total_counts;
        total_counts.resize(clusters.size() + noise.size());
        std::vector<std::unordered_map<std::string, double>> total_counts_pat;
        total_counts_pat.resize(clusters.size() + noise.size());
        for(int j=0; j < clusters.size(); j++){
            for(int k=0; k<clusters[j].size();k++){
                if(mode_local == "patterns"){
                    for (auto q = total_counts_pat_per_seq[clusters[j][k]].cbegin(),
                         end = total_counts_pat_per_seq[clusters[j][k]].cend(); q != end; ++q){
                        if(total_counts_pat[j].count(q->first) > 0){
                            total_counts_pat[j][q->first] += q->second;
                        } else{
                            total_counts_pat[j][q->first] = q->second;
                        }
                    }

                } else {
                    for (auto q = total_counts_per_seq[clusters[j][k]].cbegin(),
                         end = total_counts_per_seq[clusters[j][k]].cend(); q != end; ++q){
                        if(total_counts[j].count(q->first) > 0){
                            total_counts[j][q->first] +=  q->second;
                        } else{
                            total_counts[j][q->first] =  q->second;
                        }
                    }
                }
            }
        }
        for(int j=0; j < noise.size(); j++){
            if(mode_local == "patterns"){
                for (auto q = total_counts_pat_per_seq[noise[j]].cbegin(),
                     end = total_counts_pat_per_seq[noise[j]].cend(); q != end; ++q){
                    if(total_counts_pat[j+clusters.size()].count(q->first) > 0){
                        total_counts_pat[j+clusters.size()][q->first] += q->second;
                    } else{
                        total_counts_pat[j+clusters.size()][q->first] = q->second;
                    }
                }                
            } else {
                for (auto q = total_counts_per_seq[noise[j]].cbegin(),
                     end = total_counts_per_seq[noise[j]].cend(); q != end; ++q){
                    if(total_counts[j+clusters.size()].count(q->first) > 0){
                        total_counts[j+clusters.size()][q->first] += q->second;
                    } else{
                        total_counts[j+clusters.size()][q->first] = q->second;
                    }
                }
           }
        }
//        qDebug() << "hier 1" << clusters.size();
        for(int j =0; j< clusters.size(); j++){
            std::vector<int> myList;
            std::copy(clusters[j].begin(), clusters[j].end(), std::back_inserter(myList));
            std::vector<double> stage;
            std::vector<std::unordered_map<std::string, double>> tmp;
            if(mode_local == "patterns"){
                if(patterns_stage1and2.size() > 0){
//                    QElapsedTimer timer11;
//                    timer11.start();
                    tmp = split(patterns_stage1and2, min_events_stage, 0, stoi(patterns_stage1and2[0][2]),
                               true, myList, total_counts_pat[j], cats, cats_filter, mode_local,
                               stage1and2.size(), max_stages,0, len_seq_total);
//                    total_time_split_local += timer11.elapsed();
                } else {
                    std::unordered_map<std::string, double> empty;
                    std::unordered_map<std::string, double> no_pat;
                    no_pat["0"] = -100;
                    no_pat["1"] = -1;
                    no_pat["2"] = 0;
                    no_pat["3"] = -1;
                    tmp = {no_pat, empty, empty};
                }
            } else {
//                QElapsedTimer timer12;
//                timer12.start();
                tmp= split(stage1and2, min_events_stage, 0, 0,
                           true, myList, total_counts[j], cats, cats_filter,
                           mode_local, stage1and2.size(), max_stages,0, len_seq_total);
//                total_time_split_local += timer12.elapsed();
            }
            stage = {tmp[0]["0"], tmp[0]["1"], tmp[0]["2"]};
            stage[2] = stages[i][2];
            if(stage[0] != -100){
                if(mode_local != "patterns"){
                    stage[0] += start_ind;
                }
               //max deviation
               if(stages[i][0] - stage[0] > max_dev) {
                   stage[0] =  stages[i][0] - max_dev;
               } else if (stage[0] - stages[i][0] > max_dev){
                   stage[0] =  stages[i][0] + max_dev;
               }
            }

            //fill local stages
            for (int k=0; k < clusters[j].size(); k++){
                if(stage[0] != -100){
                    local_stages[clusters[j][k]].push_back(stage);
                } else {
                    local_stages[clusters[j][k]].push_back(stages[i]);
                }
            }
        }
//        qDebug() << "hier 2" << noise.size();
        for(int j =0; j< noise.size(); j++){
           std::vector<double> stage;
           std::vector<std::unordered_map<std::string, double>> tmp;
           if(mode_local == "patterns"){
               if(patterns_stage1and2.size() > 0){
//                   QElapsedTimer timer13;
//                   timer13.start();
                   tmp = split(patterns_stage1and2, min_events_stage, 0, stoi(patterns_stage1and2[0][2]),
                                true, {static_cast<int>(noise[j])}, total_counts_pat[j+clusters.size()],
                                cats,cats_filter, mode_local, stage1and2.size(), max_stages, 0, len_seq_total);
//                   total_time_split_local += timer13.elapsed();
               } else {
                   std::unordered_map<std::string, double> empty;
                   std::unordered_map<std::string, double> no_pat;
                   no_pat["0"] = -100;
                   no_pat["1"] = -1;
                   no_pat["2"] = 0;
                   no_pat["3"] = -1;
                   tmp = {no_pat, empty, empty};
               }
           } else {
//               QElapsedTimer timer14;
//               timer14.start();
               tmp = split(stage1and2, min_events_stage, 0, 0,
                            true, {static_cast<int>(noise[j])}, total_counts[j+clusters.size()],
                            cats,cats_filter, mode_local, stage1and2.size(), max_stages,0, len_seq_total);               
//               total_time_split_local += timer14.elapsed();
           }
           stage = {tmp[0]["0"], tmp[0]["1"], tmp[0]["2"]};
           stage[2] = stages[i][2];
            if(stage[0] != -100){
                if(mode_local != "patterns"){
                    stage[0] += start_ind;
                }
               //max deviation
               if(stages[i][0] - stage[0] > max_dev) {
                   stage[0] =  stages[i][0] - max_dev;
               } else if (stage[0] - stages[i][0] > max_dev){
                   stage[0] =  stages[i][0] + max_dev;
               }
            }

            //fill local stages
            if(stage[0] != -100){
                local_stages[noise[j]].push_back(stage);
            } else {
                local_stages[noise[j]].push_back(stages[i]);
            }
        }        
    }
    return local_stages;
}

QList<QList<QList<double>>> staging(QTableWidget* data, int num_seqs, int min_events_stage, double max_stages,
             int max_dev, QString mode, int level, std::unordered_map<std::string, double> &total_count_event, QProgressBar* bar,
                                    std::vector<std::string> &cats, std::vector<bool> &cats_filter, QString mode_local,
                                    bool run_time_test, QString path_pat, double staging_eps){
//    total_split_runs = 0;
//    total_time_split_global = 0;
//    total_time_split_local = 0;
    QElapsedTimer timer;
    timer.start();
    stages.clear();
    int r = data->rowCount() / num_seqs;
    std::vector<std::vector<std::string>> data_format;
    data_format.resize(r);
    int ind_seq = 0;
    for (int i =0; i < data->rowCount(); i++){
        if(ind_seq == r){
            ind_seq = 0;
        }
        data_format[ind_seq].push_back(data->item(i,1)->text().toStdString());
        ind_seq ++;
    }
    qDebug() << timer.elapsed();
    qDebug() << "format done";
    qDebug() << "eps:" << staging_eps;
    bar->setValue(10);
    //total pattern count
    std::unordered_map<std::string, double> total_pat_count;
    std::vector<std::vector<std::string>> patterns;
    if(mode == "patterns" || mode_local == "patterns"){
        //pattern file should be sorted on start index
        QList<QString> tmp_path = QDir::currentPath().split("/");
        QString path = "";
        for(int i=0; i<tmp_path.size()-1; i++){
            path += tmp_path[i] + "/";
        }
        QString pattern_path ="";
        if(!run_time_test){
            pattern_path = path + "pattern_file.csv";
        } else {
            pattern_path = path_pat;
        }
        QFile file(pattern_path);
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
               std::vector<std::string> pat;
               if(row.size() > 1){
                  pat.push_back(row[0].toStdString());
                  pat.push_back(row[row.size()-3].toStdString());
                  pat.push_back(row[row.size()-2].toStdString());
                  pat.push_back(row[row.size()-1].toStdString());
                  patterns.push_back(pat);

                  if(total_pat_count[row[0].toStdString()]){
                      total_pat_count[row[0].toStdString()] ++;
                  } else {
                      total_pat_count[row[0].toStdString()] = 1;
                  }
               }
        }
        qDebug() << "pattern read";
    }
    QElapsedTimer timer2;
    timer2.start();
     if(mode == "patterns"){
         get_stages_global(patterns, min_events_stage, level, max_stages, mode, 0, total_pat_count, cats, cats_filter, data_format.size(), 0, r);
     } else {
         get_stages_global(data_format, min_events_stage, level, max_stages, mode, 0, total_count_event, cats, cats_filter, data_format.size(), 0, r);
     }
    qDebug() << timer2.elapsed();
    qDebug() << "global done";

    bar->setValue(50);
    std::sort(stages.begin(), stages.end());

    bar->setValue(60);
    qDebug() << stages;
    QElapsedTimer timer3;
    timer3.start();
    std::vector<std::vector<std::vector<double>>> local_stages;
    local_stages.resize(data_format[0].size());

    //local splitting
    local_stages = local_staging(data_format, mode, min_events_stage, max_dev,
                                 local_stages, cats, cats_filter, mode_local,
                                 patterns, max_stages, r, staging_eps);
    qDebug() << timer3.elapsed();
    qDebug() << "local done";
    bar->setValue(100);

    QList<QList<QList<double>>> final;
    for (auto & high_nest : local_stages) {
        QList<QList<double>> mid_level;
        for (auto & mid_nest : high_nest) {
            QList<double> lowest_nest;
            lowest_nest.reserve(mid_nest.size());
            std::copy(mid_nest.begin(), mid_nest.end(), std::back_inserter(lowest_nest));
            mid_level.push_back(lowest_nest);
        }
        final.push_back(mid_level);
    }
// debug stagements for rune time tests
//    qDebug() << total_split_runs << "split runs";
//    qDebug() << total_time_split_global << "global split";
//    qDebug() << total_time_split_local << "local split";
//    qDebug() << total_time_split_global + total_time_split_local << "split time";

    //fixed staging comparison
//    final.clear();
//    QList<QList<double>> tmp_seq;
    //fixed staging week
//    for(int i =1; i < 31;i++){
//        double day = 4*24;
//        QList<double> tmp = {day*i, 0.0, (i-1)*1.0};
//        tmp_seq.push_back(tmp);
//    }
    //fixed staging day
//    QList<double> level_tmp = {4,3,4,2,3,4,1,4,3,4,2,4,3,4,0,4,3,4,2,4,3,4,1,4,3,4,2,4,3,4};
//    for(int i =1; i < 31;i++){
//        double day = 4*24;
//        QList<double> tmp = {day*i, 0.0, level_tmp[i-1]};
//        tmp_seq.push_back(tmp);
//    }
//    for(int i=0; i<38;i++){
//        final.push_back(tmp_seq);
//    }
//    qDebug() << final;

    return final;

}





#endif // STAGING_H
