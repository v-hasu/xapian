#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "adarank_ranker.h"
#include "scorer.h"
#include "map_scorer.h"
#include "ndcg_scorer.h"
#include "err_scorer.h"

#include <cmath>
#include <stdlib.h>

#include <fstream>

using namespace std;
using namespace Xapian;

AdaRankRanker::AdaRankRanker(){
}

AdaRankRanker::AdaRankRanker(int metric_type):Ranker(metric_type) {
}

AdaRankRanker::AdaRankRanker(int metric_type, int new_iterations){
    
    MAXPATHLEN = 200;

    this->iterations = new_iterations;

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

static double
calculateInnerProduct(vector<double> queryWeights, vector<double> weakRankerScore){

    double inner_product = 0.0;
    int query_num = queryWeights.size();

    for (int i = 0; i < query_num; ++i){
        inner_product += queryWeights[i] * weakRankerScore[i];
    }

    return inner_product;
}

static double
weightWeakRanker(double maxScore){
    return 0.5 * log( (1 + maxScore) / (1 - maxScore) );
}

double
AdaRankRanker::getMeasure(RankList ranklist, int feature_index){

    ranklist.sort_by_feature(feature_index);

    return get_score(ranklist);
}

void
AdaRankRanker::initialize(vector<Xapian::RankList> ranklists, int feature_num, int query_num) {
    
    this->queryWeights.assign(query_num,(double)1/query_num);

    for (int i = 0; i < feature_num; ++i){

        vector<double> weakRankerScore;

        for (int j = 0; j < query_num; j++){

            weakRankerScore.push_back(getMeasure(ranklists[j], i));

        }

        this->weakRankerMatrix.push_back(weakRankerScore);
    }

}

void
AdaRankRanker::createWeakranker(int feature_num){
    double maxScore = 0.0;
    double score;
    int rankerid = -1;
    double alpha;

    for(int i = 0; i < feature_num; i++){

        score = calculateInnerProduct(queryWeights,weakRankerMatrix[i]);

        if (score > maxScore){
            maxScore = score;
            rankerid = i;
        }

    }

    alpha = weightWeakRanker(maxScore);

    this->weakRankerWeights.push_back(make_pair(rankerid,alpha));
}

void
AdaRankRanker::reweightQuery(int query_num){
    double norm = 0.0;
    int weakRanker_num = this->weakRankerWeights.size();

    for(int i = 0; i < query_num; ++i){
        double query_score = 0.0;

        for (int j = 0; j < weakRanker_num; ++j){

            query_score += this->weakRankerMatrix[this->weakRankerWeights[j].first][i] * (this->weakRankerWeights[j].second);
        }

        queryWeights[i] = exp(-query_score);

        norm += queryWeights[i];
    }

    for(int i = 0; i < query_num; i++){
        queryWeights[i] /= norm;
    }
        
}

void
AdaRankRanker::batchLearning(vector<Xapian::RankList> ranklists, int feature_num, int query_num){

    createWeakranker(feature_num);

    reweightQuery(query_num);

}

void 
AdaRankRanker::train_model(){
    std::cout << "Adarank model begin to train..." << endl;

    //get the training datal
    vector<Xapian::RankList> ranklists = get_traindata();
    int query_num = ranklists.size();
    int feature_num;
    if (query_num!=0){
        vector<FeatureVector> fvv = ranklists[0].get_fvv();
        feature_num = fvv[0].get_fcount();
    }

    initialize(ranklists, feature_num, query_num);

    for(int iter_num = 0; iter_num < this->iterations; ++iter_num){

        batchLearning(ranklists, feature_num, query_num);
    }

    save_model_to_file();

}

void 
AdaRankRanker::save_model_to_file(){

	ofstream parameters_file;
    parameters_file.open("Adarank_parameters.txt");

    int parameters_size = this->weakRankerWeights.size();

    for(int i = 0; i < parameters_size; ++i) {
    	    parameters_file << weakRankerWeights[i].first << " " << weakRankerWeights[i].second <<endl;
    }
    parameters_file.close();
}

void 
AdaRankRanker::load_model_from_file(const char *parameters_file){

	vector< pair< int, double > > loaded_weakRankerWeights;

    fstream model_file (parameters_file, ios::in);
    if(!model_file.good()){
        cout << "No parameters file found"<<endl;
    }

    while (model_file.peek() != EOF) {

        int rankerid;
        double alpha;//read parameter
        model_file >> rankerid;
        model_file >> alpha;
        loaded_weakRankerWeights.push_back(make_pair(rankerid,alpha));
        model_file.ignore(100,'\n');
    }

    model_file.close();
    this->weakRankerWeights = loaded_weakRankerWeights;
}

Xapian::RankList 
AdaRankRanker::rank(Xapian::RankList & ranklist){

    vector<Xapian::FeatureVector> testfvv = ranklist.get_fvv();
    int testfvvsize = testfvv.size();

    int parameters_size = this->weakRankerWeights.size();

    for (int i = 0; i <testfvvsize; ++i){
        double adarank_score = 0.0;
        for (int j = 0; j < parameters_size; ++j){
            adarank_score += testfvv[i].fvals[weakRankerWeights[j].first] * weakRankerWeights[j].second;
        }
        testfvv[i].set_score(adarank_score);
    }

    ranklist.set_fvv(testfvv);
    ranklist.sort_by_score();

    return ranklist;
}
