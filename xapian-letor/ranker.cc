/* ranker.cc: The abstract ranker file.
 *
 * Copyright (C) 2012 Parth Gupta
 * Copyright (C) 2014 Hanxiao Sun
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */
 
#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranklist.h"
#include "ranker.h"
#include "scorer.h"
#include "map_scorer.h"
#include "ndcg_scorer.h"
#include "err_scorer.h"

#include <vector>
#include <map>
#include <algorithm>
#include <cmath>
#include <utility>
#include <stdlib.h>

using namespace std;
using namespace Xapian;

Ranker::Ranker() {
	MAXPATHLEN = 200;
}

Ranker::Ranker(int metric_type) {
	MAXPATHLEN = 200;
	switch(metric_type) {
        case 0: this -> scorer = new MAPScorer;
                break;
        case 1: this -> scorer = new NDCGScorer;
                break;
		case 2: this -> scorer = new ERRScorer;
                break;
        default: ;
    }
}

double 
Ranker::get_score(Xapian::RankList & rl){
	return this->scorer->score(rl);
}

std::vector<Xapian::RankList> 
Ranker::get_traindata(){
    return this->traindata;
}

void
Ranker::set_training_data(vector<Xapian::RankList> training_data1) {
    this->traindata = training_data1;
}

//get current working directory
std::string 
Ranker::get_cwd() {
    char temp[MAXPATHLEN];
    return (getcwd(temp, MAXPATHLEN) ? std::string(temp) : std::string(""));
}

void
Ranker::train_model() {
}

void
Ranker::save_model_to_file() {
}

void
Ranker::load_model_from_file(const std::string & model_file) {
}

Xapian::RankList
Ranker::rank(Xapian::RankList & rl) {
}

static map<string,double>
transform(Xapian::RankList &rl){

    map<string,double> blending_score;
    std::vector<FeatureVector> fvv = rl.get_fvv();
    int ranking_size = fvv.size();
    double deviation = 0.0;
    double mean = 0.0;
    double std = 0.0;
    double sumX = 0.0;
    double sumX2 = 0.0;

    for(int i =0 ; i < ranking_size; ++i){
        double score = fvv[i].get_score();
        sumX += score;
        sumX2 += score*score;
    }
    mean = (sumX/(double)ranking_size);
    deviation = (sumX2/(double)ranking_size) - (sumX/(double)ranking_size) * (sumX/(double)ranking_size);
    std = sqrt(deviation);

    for(double i = 0.0; i < ranking_size; ++i){

        double temp = fvv[i].get_score();
        blending_score[fvv[i].get_did()] = (temp-mean)/std;//1.0/(i+1.0);//ranking_size - i;

    }
    return blending_score;
}

static void
combineMap(map<string,double> & base, map<string,double> & new_add){
    if(base.size()!=new_add.size()){
        std::cout<<"the size of the ranking between each rankers are not the same"<<endl;
        exit(2);
    }
    else{
        for(map<string,double>::iterator iter = new_add.begin(); iter != new_add.end(); ++iter){
            if(base.count(iter->first)!=0){
                base[iter->first] += new_add[iter->first];
            }
            else{
                std::cout<<"the document set in different ranker are not the same"<<endl;
                exit(2);
            }
        }
    }
}

//input: several ranklist ranked by several rankers
//output: the single fused ranking esult 
Xapian::RankList
Ranker::aggregate(vector<Xapian::RankList> rls){

    int ranker_num = rls.size();

    if (0 == ranker_num){
        std::cout<<"No ranklist in rls in ranker.cc!"<<endl;
        exit(2);

    } else if(1 == ranker_num){
        return rls[0];

    } else{

        map<string,double> base = transform(rls[0]);
        for (int i = 1; i < ranker_num; ++i){
            map<string,double> new_add = transform(rls[i]);
            combineMap(base,new_add);
        }

        vector<FeatureVector> fvv = rls[0].get_fvv();
        int fvv_size = fvv.size();
        string did;
        double borda_fuse_score;
        for(int i = 0; i < fvv_size; ++i){
            did = fvv[i].get_did();
            borda_fuse_score = base[did];
            fvv[i].set_score(borda_fuse_score);
        }
        rls[0].set_fvv(fvv);
        rls[0].sort_by_score();

        return rls[0];
    }

}
