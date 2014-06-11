#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "listnet_ranker.h"

#include <cmath>

using namespace std;
using namespace Xapian;

ListNET::ListNET(){
}

/*
the equation (3) in the paper
calculates cross entropy between a "base" distribution and another one.
*/
static double
crossEntropy(std::vector<double> base, std::vector<double> predict) {

	int base_size = base.size();
	if (base_size != predict.size()){
		std::cout << "the size between base distributions and another one is not match in listnet::cossaEntropy" << endl;
	}
	double se_sum = 0;
	for(int i = 0; i < base_size; ++i){
		se_sum += base[i]*log(predict[i]);
	}

	return se_sum;
}

void 
ListNET::train_model(){

	std::cout << "ListNet model begin to train..." << endl;
    
	int iterations = 100;
	double learning_rate = 0.001;

	//get the training data
	vector<Xapian::RankList> ranklistl = get_traindata();
	int ranklistlen = ranklistl.size();

	//get feature number, it also is the number of parameters
	int feature_cnt=1;
	
	if (ranklistlen != 0){
		feature_cnt = ranklistl[0].fvv[0].get_fcount();
	}
	else{
		std::cout << "The training data in ListNet is NULL!" << endl;
	}

	//initialize the parameters for neural network 
	std::vector<double> new_parameters;
	for (int i = 1; i < feature_cnt; ++i){
		new_parameters.push_back(0.0);
	}
	
	//train
	for(int iter = 1; iter < iterations; ++iter){

		while(1/*read sample*/){
			/*cross entropy*/
			/*gradient desent*/
			/*update parameters*/
		}
	}

	//save model
	this->parameters = new_parameters;
}

void 
ListNET::save_model_to_file(){

}

void 
ListNET::load_model_from_file(const std::string & model_file){

}

Xapian::RankList 
ListNET::rank(Xapian::RankList & ranklist){

	Xapian::Scorer svm_scorer = get_scorer();

    std::vector<Xapian::FeatureVector> testfvv = ranklist.get_fvv();
    int testfvvsize = testfvv.size();

    std::vector<double> new_parameters = this->parameters;
    int parameters_size = new_parameters.size();

    for (int i = 0; i <testfvvsize; ++i){

    	int listnet_score = 0;

        map <int,double> fvals = testfvv[i].get_fvals();
        int fvalsize = fvals.size();

        if (fvalsize != parameters_size){
        	std::cout << "number of fvals don't match the number of ListNet parameters" << endl;
        }

        for(int j = 1; j <= fvalsize; ++j){                 //fvals starts from 1, not 0      
        	listnet_score += fvals[j]* new_parameters[j];      
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
