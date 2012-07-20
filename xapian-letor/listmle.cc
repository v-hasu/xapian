/* listmle.cc: The ListMLE algorithm.
 *
 * Copyright (C) 2012 Rishabh Mehrotra
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
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"
#include "listmle.h"

#include "str.h"
#include "stringutils.h"

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <math.h>

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

using namespace std;
using namespace Xapian;

//static char *line = "";
//static int max_line_len;

int MAXPATHLENTH=200;

static string get_cwd() {
    char temp[MAXPATHLENTH];
    return ( getcwd(temp, MAXPATHLENTH) ? std::string( temp ) : std::string() );
}

inline int maxPosition( vector<double> & v){
	int max = 0;
	for (unsigned int i = 1; i < v.size(); i++){
		if( v[i] > v[max])
			max = i;
	}

	return max;
}

/*ListMLE::ListMLE() {
}*/

Xapian::RankList
ListMLE::rank(const Xapian::RankList & rl) {

/*    std::map<int,double> fvals;
    std::list<FeatureVector> fvlist = rl->rl;
    std::list<FeatureVector>::const_iterator iterator;
    
    for (iterator = fvlist.begin(); iterator != fvlists.end(); ++iterator) {
	//std::cout << *iterator;
	fvals = iterator->fvals;
	
    }
*/
    return rl;
}

/*static char* readline(FILE *input) {
    int len;

    if (fgets(line, max_line_len, input) == NULL)
	return NULL;

    while (strrchr(line, '\n') == NULL) {
	max_line_len *= 2;
	line = (char *)realloc(line, max_line_len);
	len = (int)strlen(line);
	if (fgets(line + len, max_line_len - len, input) == NULL)
	    break;
    }
    return line;
}*/

void
ListMLE::learn_model() {
    printf("Learning the model..");
    string input_file_name;
    string model_file_name;
    //const char *error_msg;

    //input_file_name = get_cwd().append("/train.txt");
    model_file_name = get_cwd().append("/model.txt");
    
    /* Read the training file into the format as required by the listmle_train() function
     * with datastructure vector<vector<map<int,double>>>
     */
    vector<RankList> samples;
    //read_problem(input_file_name.c_str());
    ListMLE::parameters = listmle_train(samples);
    //ListMLE::save_model(model_file_name);  
}

double
ListMLE::absolute (double a) {
    if(a<0)
        return (-a);
    else
        return a;
}

vector<double>
ListMLE::listmle_train(vector<RankList> & samples) {
    vector<double> params;	//parameter vector
    int max_index = 19;//number of features
    //initialize 
    for( int i = 0; i < max_index; i++)
	params.push_back(0.0);

    double preLoss = 0;
    
    //learns
    while(1) {
	for(unsigned int i = 0; i < samples.size(); ++i) {
	    std::vector<FeatureVector> pairs = samples[i].get_data();;
	    int num_of_pairs = pairs.size();
	    //vector scores
	    vector<double> scores;
	    for(int j = 0; j < num_of_pairs; ++j)
	        scores.push_back( pairs[j].get_score());
			
	    int maxPos = maxPosition( scores);
	    //calculates dot product
	    vector<double> dotProducts;
	    for(int j = 0; j < num_of_pairs; ++j) {
		//dot product
		double product = 0;
		map<int,double>::const_iterator iter;				
		map<int,double> features = pairs[j].get_fvals();
		for(iter = features.begin(); iter != features.end(); ++iter) {
		    product += params[iter->first-1] * iter->second;
		}
		dotProducts.push_back(product);
	    }
	    
	    //calculate total exp of predicted score
	    double total_exp_of_predicted_score = 0;
	    for(int j = 0; j < num_of_pairs; ++j) {
		total_exp_of_predicted_score += exp( dotProducts[j]);
	    }
	    //update params
	    for(int k = 0; k < max_index; ++k) {
		//calculate delta_param				
		double delta_param = 0;
		for(int j = 0; j < num_of_pairs; ++j) {
		    if( pairs[j].get_feature_value(k+1) != 0)
			delta_param += pairs[j].get_feature_value(k+1)*exp(dotProducts[j]);										
		}
		delta_param /= total_exp_of_predicted_score;
		delta_param -= pairs[maxPos].get_feature_value( k+1);
		//update
		params[k] -= learning_rate * delta_param;
	    }	
	}

	//compute likelihood loss
	double curLoss = 0;
	bool isStop = false;
	for(unsigned int i = 0; i < samples.size(); ++i) {
	    vector<FeatureVector> pairs = samples[i].get_data();
	    int num_of_pairs = pairs.size();
	    //vector scores
	    vector<double> scores;
	    for(int j = 0; j < num_of_pairs; ++j)
		scores.push_back( pairs[j].get_score());
	
	    int maxPos = maxPosition(scores);
	    //calculates dot product
	    vector<double> dotProducts;
	    for(int j = 0; j < num_of_pairs; ++j) {
		//dot product
		double product = 0;
		map<int,double>::const_iterator iter;				
		map<int,double> features = pairs[j].get_fvals();
		
		for(iter = features.begin(); iter != features.end(); ++iter) {
		    product += params[iter->first-1] * iter->second;
	        }
		dotProducts.push_back(product);
	    }
	    //calculate total exp of predicted score
	    double total_exp_of_predicted_score = 0;
	    for(int j = 0; j < num_of_pairs; ++j) {
		total_exp_of_predicted_score += exp( dotProducts[j]);
	    }
	    
	    curLoss += log( total_exp_of_predicted_score);
	    curLoss -= dotProducts[maxPos];
        }
        
        cout<<"Tolerance rate: "<<absolute( curLoss - preLoss)<<endl;
        if( absolute(curLoss - preLoss) < tolerance_rate) {
	    isStop = true;
    	    break;
        }
	else
	    preLoss = curLoss;
	
	//break while
	if( isStop)
	    break;
    }
    return params;
}

void 
ListMLE::load_model(const std::string & /*model_file*/) {
}

void 
ListMLE::save_model() {
    ofstream train_file;
    train_file.open("model.txt");
    int num_param=parameters.size();

    for (int i=0; i<num_param; ++i)
	train_file << parameters[i];
    
    train_file.close();
}

double 
ListMLE::score(const Xapian::FeatureVector & /*fv*/) {
    return 0.0;
}

