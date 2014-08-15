/* ranker.cc: The abstract ranker file.
 *
 * Copyright (C) 2012 Parth Gupta
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
#include <utility>
#include <stdlib.h>

using namespace std;
using namespace Xapian;

struct scoreComparer {
    bool operator()(const pair<Xapian::docid, int>& first_pair, const pair<Xapian::docid, int>& second_pair) const {
        return first_pair.second > second_pair.second;
    }
};

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

/*Xapian::Scorer 
Ranker::get_scorer(){
    return this->scorer;
}*/

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
/*
static map<Xapian::docid,int>
transform(Xapian::RankList &rl){
    map<Xapian::docid,int> borda_score;
    std::vector<FeatureVector> fvv = rl.get_fvv();
    int ranking_size = fvv.size();
    for(int i = 0; i < ranking_size; ++i){
        //the score definition is the fisrt doc get the maximum score, which is just the size of the doc set.
        borda_score[fvv[i].get_did()] = ranking_size-i;
    }
    return borda_score;
}

static void
combineMap(map<Xapian::docid,int> base, map<Xapian::docid,int> & new_add){
    if(base.size()!=new_add.size()){
        std::cout<<"the size of the ranking between each rankers are not the same"<<endl;
        exit(2);
    }
    else{
        for(map<Xapian::docid,int>::iterator iter = new_add.begin(); iter != new_add.end(); ++iter){
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

//use borda-fuse
std::vector<Xapian::docid>
Ranker::aggregate(std::vector<Xapian::RankList> rls){
    
    vector<Xapian::docid> ranking_result;

    int ranker_num = rls.size();
    if (1 == ranker_num){
        std::vector<FeatureVector> fvv = rls[0].get_fvv();
        for(std::vector<FeatureVector>::iterator iter = fvv.begin(); iter != fvv.end(); ++iter)
            ranking_result.push_back(iter->did);
    }
    else if (0 == ranker_num){
        std::cout<<"No ranklist in rls in ranker.cc!"<<endl;
        exit(2);
    }
    else{
        map<Xapian::docid,int> base = transform(rls[0]);
        for (int i = 1; i < ranker_num; ++i){
            map<Xapian::docid,int> new_add = transform(rls[i]);
            combineMap(base,new_add);
        }

        vector<pair<Xapian::docid, int> > aggregate_score_vec(base.begin(), base.end());  
        sort(aggregate_score_vec.begin(), aggregate_score_vec.end(), scoreComparer());

        for(vector<pair<Xapian::docid, int> >::iterator iter = aggregate_score_vec.begin(); iter != aggregate_score_vec.begin(); ++iter){
            ranking_result.push_back(iter->first);
        }

    }

    return ranking_result;

}*/