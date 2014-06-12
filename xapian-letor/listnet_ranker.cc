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
	vector<Xapian::RankList> ranklists = get_traindata();
	int ranklist_len = ranklists.size();

	//get feature number, it also is the number of parameters
	int feature_cnt = -1;
	
	if (ranklist_len != 0){
		feature_cnt = ranklists[0].fvv[0].get_fcount();
	}
	else{
		std::cout << "The training data in ListNet is NULL!" << endl;
		//exit(1);
	}

	//initialize the parameters for neural network 
	std::vector<double> new_parameters;
	for (int feature_num = 1; feature_num < feature_cnt; ++feature_num){
		new_parameters.push_back(0.0);
	}
	
	//iterations
	for(int iter = 1; iter < iterations; ++iter){

		//for each sample in the training set
		for(int sample_num = 0; sample_num < ranklist_len; ++sample_num){
			//get the features for special sample 
			std::vector<FeatureVector> feature_vectors = ranklists[sample_num].get_fvv();
			int feature_vectors_len = feature_vectors.size();
		
			double exponent_sum = 0;//corresponding the z in the paper
			vector<double> dot_products;//corresponding the y in the paper
			double predicted_score_exponent = 0;
			double temp_dot_product;

			//for each feature vector in special ranklist
			for(int feature_vectors_num = 0; feature_vectors_num < feature_vectors_len; ++feature_vectors_num){

				exponent_sum += exp(feature_vectors[feature_vectors_num].get_score());
				
				map<int,double> feature_sets = feature_vectors[feature_vectors_num].get_fvals();
				temp_dot_product = 0;
				//for each feature in special feature vector
				for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){
					temp_dot_product += new_parameters[iter->first-1] * iter->second;
				}

				dot_products.push_back(temp_dot_product);
			}

			for(int feature_vectors_num = 0; feature_vectors_num < feature_vectors_len; ++feature_vectors_num){
				predicted_score_exponent += exp(dot_products[feature_vectors_num]);
			}


			for(int feature_num=0; feature_num < feature_cnt; ++feature_num) {

                double delta_w = 0;

                for(int feature_vectors_num=0; feature_vectors_num < feature_vectors_len; ++feature_vectors_num) {

                    //feature start from 1
                    if( feature_vectors[feature_vectors_num].get_feature_value(feature_num+1) != 0) {

                        //the equation (6) in the paper
                        delta_w -= (exp( feature_vectors[feature_vectors_num].get_score())/exponent_sum)*feature_vectors[feature_vectors_num].get_feature_value(feature_vectors_num+1);                   
                        delta_w +=  (1/predicted_score_exponent)*exp(dot_products[feature_vectors_num])*feature_vectors[feature_vectors_num].get_feature_value(feature_vectors_num+1);
                    }
                }
            //multiply learning rate 
            new_parameters[feature_num] -= learning_rate * delta_w;
            }


		}


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
