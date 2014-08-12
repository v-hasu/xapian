#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "listmle_ranker.h"

#include <algorithm>
#include <cmath>
#include <stdlib.h>
#include <fstream>

using namespace std;
using namespace Xapian;

ListMLERanker::ListMLERanker(){
}

ListMLERanker::ListMLERanker(int metric_type):Ranker(metric_type) {
}

static double
calculateInnerProduct(vector<double> parameters, map<int,double> feature_sets){

	double inner_product = 0.0;

	for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){
		inner_product += parameters[iter->first] * iter->second;//the feature start from 1, while the parameters strat from 0
	}

	return inner_product;
}

/*
loss function: negative log Likelihood
the equation (9) in the paper
*/
static double
negativeLogLikelihood(vector<FeatureVector> sorted_feature_vectors, vector<double> new_parameters) {

	int list_length = sorted_feature_vectors.size();
	//top 1 probability
	double first_place_in_ground_truth;
	double expsum = 0.0;
	
	if (list_length!=0){
		//also use top 1 probability, so we need the fisrt one in the groud truth
		first_place_in_ground_truth = exp(calculateInnerProduct(new_parameters,sorted_feature_vectors[0].get_fvals()));//should be put into init?
	}
	else{
		std::cout << "the feature_vectors is null in ListMLERanker:logLikelihood" << endl;
		exit(1);
	}
	
	for(int i = 0; i < list_length; i++){
		expsum += exp(calculateInnerProduct(new_parameters,sorted_feature_vectors[i].get_fvals()));
	}

	return log(expsum)-log(first_place_in_ground_truth);
}

static vector<double>
calculateGradient(vector<FeatureVector> sorted_feature_vectors, vector<double> new_parameters) {

	vector<double> gradient(sorted_feature_vectors[0].get_fcount(),0);//need to be optimized,how to get the length of the features

	int list_length = sorted_feature_vectors.size();

	vector<double> exponents;
	double expsum = 0.0;
	
	for(int i = 0; i < list_length; i++){
		double temp = exp(calculateInnerProduct(new_parameters,sorted_feature_vectors[i].get_fvals()));
		exponents.push_back(temp);
		expsum += temp;
	}

	for(int i = 0; i < list_length; i++){//not convenient, but in order to get the feature, I have to run the ranklist loop first. need to see whether the fvals could be a vector
		map<int,double> feature_sets = sorted_feature_vectors[i].get_fvals();

		//for each feature in special feature vector
		for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){

			gradient[iter->first] += iter->second * exponents[i]/ expsum;

		}
		
	}

	map<int,double> first_place_in_ground_truth_feature_sets = sorted_feature_vectors[0].get_fvals();

	for (map<int,double>::iterator iter = first_place_in_ground_truth_feature_sets.begin(); iter != first_place_in_ground_truth_feature_sets.end(); ++iter){

			gradient[iter->first] -= iter->second;

		}

	return gradient;
}

static void
updateParameters(vector<double> & new_parameters, vector<double> gradient, double learning_rate){
	int num = new_parameters.size();
	if (num != gradient.size()){
		cout << "the size between base new_parameters and gradient is not match in listnet::updateParameters" << endl;
		cout << "the size of new_parameters: " << num << endl;
		cout << "the size of gradient: " << gradient.size() << endl;
	}
	else{
		for (int i = 0; i < num; i++){
			new_parameters[i] -= gradient[i] * learning_rate;
		}
	}
}

static void
batchLearning(RankList ranklists, vector<double> & new_parameters, double learning_rate){

	//ListMLE need to use the ground truth, so sort before train 
	ranklists.sort_by_label();
	//get feature vectors
	vector<FeatureVector> feature_vectors = ranklists.get_fvv();

	//compute gradient
	vector<double> gradient = calculateGradient(feature_vectors, new_parameters);

	//update parameters: w = w - gradient * learningRate
	updateParameters(new_parameters, gradient, learning_rate);

}

void 
ListMLERanker::train_model(){
	
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
		exit(1);
	}

	//initialize the parameters for neural network 
	std::vector<double> new_parameters;
	for (int feature_num = 0; feature_num < feature_cnt; ++feature_num){
		new_parameters.push_back(0.0);
	}
	
	//iterations
	for(int iter_num = 0; iter_num < iterations; ++iter_num){

		for(int sample_num = 0; sample_num < ranklist_len; ++sample_num){
			
			batchLearning(ranklists[sample_num], new_parameters, learning_rate);

		}
	}

	this->parameters = new_parameters;
}

void 
ListMLERanker::save_model_to_file(){

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
ListMLERanker::load_model_from_file(const char *parameters_file){

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
ListMLERanker::rank(Xapian::RankList & ranklist){


    std::vector<Xapian::FeatureVector> testfvv = ranklist.get_fvv();
    int testfvvsize = testfvv.size();

    std::vector<double> new_parameters = this->parameters;
    int parameters_size = new_parameters.size();

    // for (int i = 0; i < parameters_size; ++i){
    // 	cout << "new_parameters[" << i << "]: " << new_parameters[i] << endl;
    // }

    for (int i = 0; i <testfvvsize; ++i){

    	double listmle_score = 0;

        map <int,double> fvals = testfvv[i].get_fvals();
        int fval_size = fvals.size();

        if (fval_size != parameters_size){//fval start from 1, while the parameters start from 1
        	cout << "number of fvals don't match the number of ListNet parameters" << endl;
        	cout << "the size of parameters: " << parameters_size << endl;
			cout << "the size of fvals: " << fval_size << endl;
        }

        for(int j = 0; j < fval_size; ++j){                 //fvals starts from 1, not 0      
        	listmle_score += fvals[j] * new_parameters[j];   
        	// cout << "fvals[" << j << "]: " << fvals[j] << endl;
        	// cout << "new_parameters[" << j << "]: " << new_parameters[j] << endl;
        	// cout << "listnet_score: " << listnet_score << endl;
        } 

        testfvv[i].set_score(listmle_score);

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