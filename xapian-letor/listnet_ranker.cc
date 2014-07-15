#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "listnet_ranker.h"

#include <cmath>
#include <stdlib.h>

#include <fstream>

using namespace std;
using namespace Xapian;

ListNETRanker::ListNETRanker(){
}

ListNETRanker::ListNETRanker(int metric_type):Ranker(metric_type) {
}

static double 
calculateInnerProduct(vector<double> parameters, map<int,double> feature_sets){

	double inner_product = 0.0;

	for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){
		inner_product += parameters[iter->first-1] * iter->second;//the feature start from 1, while the parameters strat from 0
	}

	return inner_product;
}

/*
loss function: cross Entropy
the equation (3) in the paper
*/
static double
crossEntropy(std::vector<double> groundTruth, std::vector<double> predict) {

	int list_length = groundTruth.size();
	if (list_length != predict.size()){
		std::cout << "the size between ground Truth distribution and predict score distribution is not match in listnet::cossaEntropy" << endl;
	}
	double crossEntropy = 0;
	for(int i = 0; i < list_length; ++i){
		crossEntropy += groundTruth[i]*log(predict[i]);
	}

	return crossEntropy;
}

/*
Top k probability
the THeorem (8) in the paper(with k = 1)
only work for binomial relevance
*/
static vector< vector<double> >
initializeProbability(vector<FeatureVector> feature_vectors, vector<double> new_parameters) {

	int list_length = feature_vectors.size();

	//probability distribution, y is ground truth, while z is predict score
	vector<double> prob_y;
	vector<double> prob_z;
	double expsum_y=0,expsum_z=0;

	for(int i = 0; i < list_length; i++){
		if (feature_vectors[i].get_label()==0){
			expsum_y += exp(-1);
		}
		else{
			expsum_y += exp(1);
		}
		expsum_z += exp(calculateInnerProduct(new_parameters,feature_vectors[i].get_fvals()));
	}

	for(int i = 0; i < list_length; i++){
		if (feature_vectors[i].get_label()==0){
			prob_y.push_back(exp(-1)/expsum_y);
		}
		else{
			prob_y.push_back(exp(1)/expsum_y);
		}
		prob_z.push_back(exp(calculateInnerProduct(new_parameters,feature_vectors[i].get_fvals()))/expsum_z);
	}

	vector< vector<double> > prob;
	prob.push_back(prob_y);
	prob.push_back(prob_z);

	return prob;
}

/*
calculate the gradient for updating parameter 
the equation (6) in the paper
*/
static vector<double>
calculateGradient(vector<FeatureVector> feature_vectors, vector< vector<double> > prob) {

	vector<double> gradient(feature_vectors[0].get_fcount()-1,0);//need to be optimized,how to get the length of the features?
	int list_length = feature_vectors.size();
	
	vector<double> prob_y(prob[0]);
	vector<double> prob_z(prob[1]);

	for(int i = 0; i < list_length; i++){
		map<int,double> feature_sets = feature_vectors[i].get_fvals();

		//for each feature in special feature vector
		for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){

			//the first term in the serivation, the equation (6) in the paper
			double first_term = - prob_y[i] * iter->second;//the feature start from 1, while the parameters strat from 0

			gradient[iter->first-1] += first_term;//not i
			//std::cout << "first_term: " << first_term << endl;

			//the second term in the serivation, the equation (6) in the paper
			double second_term = prob_z[i] * iter->second;//the feature start from 1, while the parameters strat from 0
			gradient[iter->first-1] += second_term;
			//std::cout << "gradient["<< iter->first-1 << "]: " << gradient[iter->first-1] << endl;

		}
	}

	return gradient;
}

static void
updateParameters(vector<double> new_parameters, vector<double> gradient, double learning_rate){
	int num = new_parameters.size();
	if (num != gradient.size()){
		std::cout << "the size between base new_parameters and gradient is not match in listnet::updateParameters" << endl;
		std::cout << "the size of new_parameters: " << num << endl;
		std::cout << "the size of gradient: " << gradient.size() << endl;
	}
	else{
		for (int i = 0; i < num; i++){
			new_parameters[i] -= gradient[i] * learning_rate;
			// std::cout << "new_parameters: " << new_parameters[i]<< endl;
			// std::cout << "gradient: " << gradient[i]<< endl;
			// std::cout << "learning_rate: " << learning_rate<< endl;
		}
	}
}

static void
batchLearning(RankList ranklists, vector<double> new_parameters, double learning_rate){

	//get feature vectors
	vector<FeatureVector> feature_vectors = ranklists.get_fvv();
	//initialize Probability distributions of y and z
	vector< vector<double> > prob = initializeProbability(feature_vectors, new_parameters);

	//compute gradient
	vector<double> gradient = calculateGradient(feature_vectors, prob);

	//update parameters: w = w - gradient * learningRate
	updateParameters(new_parameters, gradient, learning_rate);

	//std::cout << crossEntropy(prob[0], prob[1]) << endl;

}

void 
ListNETRanker::train_model(){

	std::cout << "ListNet model begin to train..." << endl;
    
    //should be optimized, the parameter should could be setting in the main function
	int iterations = 10;
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
		exit(1);
	}

	//initialize the parameters for neural network 
	vector<double> new_parameters(feature_cnt-1, 0.0);//see FeatureManager::transform, the feature start from 1, so need to add 1, need 

	//iterations
	for(int iter_num = 1; iter_num < iterations; ++iter_num){

		for(int sample_num = 0; sample_num < ranklist_len; ++sample_num){
			
			batchLearning(ranklists[sample_num], new_parameters, learning_rate);

		}
	}

	this->parameters = new_parameters;
}

void 
ListNETRanker::save_model_to_file(){

    vector<double> trained_parameters = this->parameters;

	ofstream parameters_file;
    parameters_file.open("ListNet_parameters.txt");

    int parameters_size = trained_parameters.size();

    for(int i = 0; i < parameters_size; ++i) {
    	    parameters_file << trained_parameters[i] <<endl;
    }
    parameters_file.close();
}

void 
ListNETRanker::load_model_from_file(const char *parameters_file){

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
ListNETRanker::rank(Xapian::RankList & ranklist){

	//Xapian::Scorer svm_scorer = get_scorer();

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
