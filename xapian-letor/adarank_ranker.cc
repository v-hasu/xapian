#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "adarank_ranker.h"

#include <cmath>
#include <stdlib.h>

#include <fstream>

using namespace std;
using namespace Xapian;

AdaRankRanker::AdaRankRanker(){
}

AdaRankRanker::AdaRankRanker(int metric_type):Ranker(metric_type) {
}

static double
calculateInnerProduct(vector<double> queryWeights, vector<double> weakRankers){

    double inner_product = 0.0;
    int  size = queryWeights.size();

    for (int i = 0; i < size; ++i){
        inner_product += queryWeights[i] * weakRankers[i];
    }

    return inner_product;
}

double
AdaRankRanker::getMeasure(Xapian::RankList ranklist){

    ranklist.sort_by_label();

    return get_score(ranklist);
}

void
AdaRankRanker::initialize(vector<Xapian::RankList> ranklists, int feature_len, int ranklist_len) {

    //need to optimize the process to get the feature length of the training set
    

    this->queryWeights.assign(ranklist_len,1/ranklist_len);

    for (int i = 0; i < ranklist_len; ++i){

        vector<double> weakRankerScore;

        for (int j = 0; j < feature_len; j++){
            weakRankerScore.push_back(getMeasure(ranklists[i]));
        }

        this->weakRankerMatrix.push_back(weakRankerScore);
    }

}

static double
weightWeak(double maxScore){
    return 0.5 * log((1+maxScore)/(1-maxScore));
}


void
AdaRankRanker::weakranker(int feature_len){
    double maxScore = 0;
    double score;
    int rankerid = -1;
    double alpha;

    for(int i = 0; i < feature_len; i++){
        score = calculateInnerProduct(queryWeights,weakRankerMatrix[i]);
        if (score > maxScore){
            maxScore = score;
            rankerid = i;
        }
    }

    alpha = weightWeak(maxScore);

    this->weakRankerWeights.push_back(make_pair(rankerid,alpha*maxScore));
}

void
AdaRankRanker::reweightQuery(){
    double norm = 0;
    int len = this->weakRankerWeights.size();

    for(int i = 0; i < len; i++){
        queryWeights[i] = exp(-weakRankerWeights[i].second);
        norm += queryWeights[i] ;
    }

    for(int i = 0; i < len; i++)
        queryWeights[i] /= norm;
}

void
AdaRankRanker::batchLearning(vector<Xapian::RankList> ranklists, int feature_len, int ranklist_len){

    weakranker(feature_len);
    reweightQuery();

}

void 
AdaRankRanker::train_model(){
    std::cout << "Adarank model begin to train..." << endl;
    
    //should be optimized, the parameter should could be setting in the main function
    int iterations = 10;

    //get the training data
    vector<Xapian::RankList> ranklists = get_traindata();
    int ranklist_len = ranklists.size();
    int feature_len;
    if (ranklist_len!=0){
        vector<FeatureVector> fvv = ranklists[0].get_fvv();
        feature_len = fvv[0].get_fcount();
    }

    initialize(ranklists, feature_len, ranklist_len);

    for(int iter_num = 1; iter_num < iterations; ++iter_num){
        batchLearning(ranklists, feature_len, ranklist_len);
    }

}

void 
AdaRankRanker::save_model_to_file(){

    vector<double> trained_parameters = this->parameters;

	ofstream parameters_file;
    parameters_file.open("Adarank_parameters.txt");

    int parameters_size = trained_parameters.size();

    for(int i = 0; i < parameters_size; ++i) {
    	    parameters_file << trained_parameters[i] <<endl;
    }
    parameters_file.close();
}

void 
AdaRankRanker::load_model_from_file(const char *parameters_file){

	vector<double> loaded_parameters;

    fstream train_file (parameters_file, ios::in);
    if(!train_file.good()){
        cout << "No parameters file found"<<endl;
    }

    while (train_file.peek() != EOF) {

        double parameter;//read parameter
        train_file >> parameter;
        loaded_parameters.push_back(parameter);
        
    }

    train_file.close();
    this->parameters = loaded_parameters;
}

Xapian::RankList 
AdaRankRanker::rank(Xapian::RankList & ranklist){

    std::vector<Xapian::FeatureVector> testfvv = ranklist.get_fvv();
    int testfvvsize = testfvv.size();

    std::vector<double> new_parameters = this->parameters;
    int parameters_size = new_parameters.size();

    for (int i = 0; i <testfvvsize; ++i){

    	int listnet_score = 0;

        map <int,double> fvals = testfvv[i].get_fvals();
        int fvalsize = fvals.size();

        if (fvalsize != parameters_size+1){//fval start from 1, while the parameters start from 1
        	std::cout << "number of fvals don't match the number of ListNet parameters" << endl;
        }

        for(int j = 1; j < fvalsize; ++j){                 //fvals starts from 1, not 0      
        	listnet_score += fvals[j]* new_parameters[j-1];      
        } 

        testfvv[i].set_score(listnet_score);

    }

    ranklist.set_fvv(testfvv);
    ranklist.sort_by_score();
/*      
    std::vector<double> scores;  
    std::cout << "NDCG: " << svm_scorer.ndcg_scorer(ranklist) << endl;
    std::cout << "ERR: " << svm_scorer.err_scorer(ranklist) << endl;
*/

    return ranklist;
}
