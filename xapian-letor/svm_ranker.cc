/* svmranker.cc The RankerSVM algorithm.
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
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "svm_ranker.h"

#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <map>

#include <algorithm>

#include <libsvm/svm.h>

using namespace std;
using namespace Xapian;

//libsvm API 
struct svm_parameter param;
struct svm_problem prob;
struct svm_model * trainmodel;
struct svm_node * test;

/*static string get_cwd() {
    char temp[200];
    return (getcwd(temp, 200) ? std::string(temp) : std::string());
}*/

SVMRanker::SVMRanker() {
}

SVMRanker::SVMRanker(int metric_type):Ranker(metric_type) {
}

void
SVMRanker::train_model() {
    std::cout << "SVM model begin to train..."<<endl;

    param.svm_type = 4;                                         //nu-SVR 
    param.kernel_type = 0;                                      //linear Kernel
    param.degree = 3;                                           //parameter for poly Kernel, default 3
    param.gamma = 0;                                            //parameter for poly/rbf/sigmoid Kernel, default 1/num_features
    param.coef0 = 0;                                            //parameter for poly/sigmoid Kernel, default 0
    param.nu = 0.5;                                             //parameter for nu-SVC/one-class SVM/nu-SVR, default 0.5
    param.cache_size = 100;                                     //default 40mb
    param.C = 1;                                                //penalty parameter, default 1
    param.eps = 1e-3;                                           //stopping criteria, default 1e-3
    param.p = 0.1;                                              //parameter for e -SVR, default 0.1
    param.shrinking = 1;                                        //use the shrinking heuristics
    param.probability = 0;                                      //probability estimates
    param.nr_weight = 0;                                        //parameter for C-SVCl
    param.weight_label = NULL;                                  //parameter flor C-SVC
    param.weight = NULL;                                        //parameter for c-SVC

    vector<Xapian::RankList> ranklistl = get_traindata();       //use list to run?
    int ranklistlenl = ranklistl.size();

    //set the parameter for svm_problem
    prob.l = 0;                                                 //the sum of the sample point, for each feature vector in the ranklist
    for(int svmnum = 0; svmnum < ranklistlenl; ++svmnum){
        prob.l += ranklistl[svmnum].fvv.size();
    }
    prob.x = new svm_node* [prob.l];                            //feature vector
    prob.y = new double [prob.l];                               

    cout << "libsvm set OK!"<<endl;

    //sparse storage, libsvm just need the non-zore feature
    int flag = 0;
    for (int i = 0; i < ranklistlenl; ++i){                      //ranklist List
        vector<Xapian::FeatureVector> trainfvv = ranklistl[i].get_fvv();
        
        int trainfvvsize = trainfvv.size();
        for(int j = 0; j < trainfvvsize; ++j ){                  //single ranklist
            prob.x[flag] = new svm_node [trainfvv[j].get_nonzero_num()+1];    //need one more space for the last default -1
            
            int fvflag = 0;
            map <int,double> trainfvals = trainfvv[j].get_fvals();
            int fvalsize = trainfvals.size();
            for(int k = 1; k <= fvalsize; ++k){                  //feature vector, fv's index start from 1 so we need =
                double fvalue = trainfvals[k];
                if(fvalue != 0){
                    prob.x[flag][fvflag].index = k;
                    prob.x[flag][fvflag].value = fvalue;
                    //cout << "fvalue: " << fvalue <<endl;
                    fvflag++;
                }
            }
            prob.x[flag][fvflag].index = -1;
            prob.x[flag][fvflag].value = -1;
            //cout << "fvflag: " << fvflag <<endl<<endl;

            prob.y[flag] = trainfvv[j].get_label();
            flag++;
        }
    }
    //cout << "flag: " << flag <<endl;

    //printf("Learning the model..");
    //string input_file_name;
    string model_file_name;
    const char *error_msg;

    //input_file_name = get_cwd().append("/train.txt");
    //model_file_name = get_cwd().append("/model.txt");

    //read_problem(input_file_name.c_str());
    error_msg = svm_check_parameter(&prob, &param);
    if (error_msg) {
        fprintf(stderr, "svm_check_parameter failed: %s\n", error_msg);
        //exit(1);
    }

    trainmodel = svm_train(&prob, &param);
    this->model = trainmodel;
    /*
    if (svm_save_model(model_file_name.c_str(), trainmodel)) {
        fprintf(stderr, "can't save model to file %s\n", model_file_name.c_str());
        exit(1);
    }
    else{
        cout << "svm model saved successfully" << endl;
    }
    */
}

void 
SVMRanker::save_model_to_file(){

    string model_file_name;

    model_file_name = get_cwd().append("/model.txt");

    trainmodel = this->model;

    if (svm_save_model(model_file_name.c_str(), trainmodel)) {
        fprintf(stderr, "can't save model to file %s\n", model_file_name.c_str());
    }
    else{
        cout << "svm model saved successfully" << endl;
    }
}

void 
SVMRanker::load_model_from_file(const std::string & model_file){
    this->model = svm_load_model(model_file.c_str());
}

Xapian::RankList
SVMRanker::rank(Xapian::RankList & ranklist){

    //Xapian::Scorer svm_scorer = get_scorer();
    std::vector<Xapian::FeatureVector> testfvv = ranklist.get_fvv();

    int testnonzero;
    double svmscore;
    int testfvvsize = testfvv.size();

    for (int i = 0; i <testfvvsize; ++i){
        
        testnonzero = testfvv[i].get_nonzero_num();
        test = new svm_node [testnonzero+1];

        int flag = 0;
        map <int,double> fvals = testfvv[i].get_fvals();
        int fvalsize = fvals.size();
        for(int j = 1; j <= fvalsize; ++j){                 //fvals starts from 1, not 0             
            if(fvals[j] != 0){
                test[flag].index = j;
                test[flag].value = fvals[j];
                flag++;
            }
        } 

        test[flag].index = -1;
        test[flag].value = -1;

        svmscore = svm_predict(this->model,test);

        testfvv[i].set_score(svmscore);

    }

    ranklist.set_fvv(testfvv);
    ranklist.sort_by_score();
/*      
    std::vector<double> scores;  
    std::cout << "NDCG: " << svm_scorer.ndcg_scorer(ranklist) << endl;
    std::cout << "ERR: " << svm_scorer.err_scorer(ranklist) << endl;

    std::cout << "ERR: " << get_score(ranklist) << endl;
*/
    return ranklist;
}