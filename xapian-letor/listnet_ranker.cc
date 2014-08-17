/* svmranker.cc The listnet algorithm.
 * 
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

#include "ranker.h"
#include "ranklist.h"
#include "listnet_ranker.h"
#include "scorer.h"
#include "map_scorer.h"
#include "ndcg_scorer.h"
#include "err_scorer.h"

#include <cmath>
#include <stdlib.h>

#include <fstream>

using namespace std;
using namespace Xapian;

ListNETRanker::ListNETRanker(){
}

ListNETRanker::ListNETRanker(int metric_type):Ranker(metric_type) {
}

ListNETRanker::ListNETRanker(int metric_type, int new_iterations, double new_learning_rate){
	
	MAXPATHLEN = 200;

	this->iterations = new_iterations;

	this->learning_rate = new_learning_rate;

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
calculateInnerProduct(vector<double> parameters, map<int,double> feature_sets){

	double inner_product = 0.0;

	for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){
		inner_product += parameters[iter->first] * iter->second;
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
		expsum_y += exp(feature_vectors[i].get_label());
		expsum_z += exp(calculateInnerProduct(new_parameters,feature_vectors[i].get_fvals()));
	}

	for(int i = 0; i < list_length; i++){
		prob_y.push_back(exp(feature_vectors[i].get_label())/expsum_y);
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

	vector<double> gradient(feature_vectors[0].get_fcount(),0);//need to be optimized,how to get the length of the features?
	int list_length = feature_vectors.size();
	
	vector<double> prob_y(prob[0]);
	vector<double> prob_z(prob[1]);

	for(int i = 0; i < list_length; i++){
		map<int,double> feature_sets = feature_vectors[i].get_fvals();

		//for each feature in special feature vector
		for (map<int,double>::iterator iter = feature_sets.begin(); iter != feature_sets.end(); ++iter){

			//the first term in the serivation, the equation (6) in the paper
			double first_term = - prob_y[i] * iter->second;//the feature start from 1, while the parameters strat from 0

			gradient[iter->first] += first_term;//not i
			
			//the second term in the serivation, the equation (6) in the paper
			double second_term = prob_z[i] * iter->second;//the feature start from 1, while the parameters strat from 0
			gradient[iter->first] += second_term;

		}
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

			double temp = new_parameters[i];
			new_parameters[i] = temp - (gradient[i] * learning_rate);	
		}
	}
}

static void
batchLearning(RankList & ranklists, vector<double> & new_parameters, double learning_rate){

	//get feature vectors
	vector<FeatureVector> feature_vectors = ranklists.get_fvv();
	//initialize Probability distributions of y and z
	vector< vector<double> > prob = initializeProbability(feature_vectors, new_parameters);

	//compute gradient
	vector<double> gradient = calculateGradient(feature_vectors, prob);

	//update parameters: w = w - gradient * learningRate
	updateParameters(new_parameters, gradient, learning_rate);

}

void 
ListNETRanker::train_model(){

	std::cout << "ListNet model begin to train..." << endl;
    
    //should be optimized, the parameter should could be setting in the main function

	//get the training data
	vector<Xapian::RankList> ranklists = get_traindata();
	int ranklist_len = ranklists.size();

	//get feature number, it also is the number of parameters
	int feature_cnt = -1;
	if (ranklist_len != 0){
		feature_cnt = ranklists[0].fvv[0].get_fcount();
	}
	else{
		cout << "The training data in ListNet is NULL!" << endl;
		exit(1);
	}

	//initialize the parameters for neural network 
	vector<double> new_parameters(feature_cnt, 0.0);//see FeatureManager::transform, the feature start from 1, so need to add 1, need 

	//iterations
	for(int iter_num = 0; iter_num < this->iterations; ++iter_num){

		for(int sample_num = 0; sample_num < ranklist_len; ++sample_num){

			batchLearning(ranklists[sample_num], new_parameters, this->learning_rate);

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


    std::vector<Xapian::FeatureVector> testfvv = ranklist.get_fvv();
    int testfvvsize = testfvv.size();

    std::vector<double> new_parameters = this->parameters;
    int parameters_size = new_parameters.size();

    for (int i = 0; i <testfvvsize; ++i){

    	double listnet_score = 0;

        map <int,double> fvals = testfvv[i].get_fvals();
        int fval_size = fvals.size();

        if (fval_size != parameters_size){//fval start from 1, while the parameters start from 1
        	cout << "number of fvals don't match the number of ListNet parameters" << endl;
        	cout << "the size of parameters: " << parameters_size << endl;
			cout << "the size of fvals: " << fval_size << endl;

        }

        for(int j = 0; j < fval_size; ++j){                 //fvals starts from 1, not 0      
        	listnet_score += fvals[j] * new_parameters[j];   

        } 

        testfvv[i].set_score(listnet_score);

    }

    ranklist.set_fvv(testfvv);
    ranklist.sort_by_score();

    return ranklist;
}
