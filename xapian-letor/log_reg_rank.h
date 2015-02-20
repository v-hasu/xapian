#ifndef LOGREGRANK_H
#define LOGREGRANK_H


#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"

#include <list>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;
//typedef map<int,double> tuple;
//typedef vector<tuple> instance;
//typedef vector<double> scores;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Log_Reg: public Ranker {

  public:
    string models;
    vector<double> parameters;
    int number_of_epochs; 
    //double tolerance_rate;
    double learning_rate;
    vector<Xapian::RankList> training_data;
    //vector<scores> all_tuple_scores;
    
  public:
    Log_Reg() {
	number_of_epochs=15;
	learning_rate=0.5;	
	};


    /* Override all the four methods below in the ranker sub-classes files
     * wiz listmle.cc , listnet.cc, listmle.cc and so on
     */
    Xapian::RankList rank(const Xapian::RankList rlist);
    
    void set_training_data(vector<Xapian::RankList> training_data1);

    void learn_model();

    void load_model(const std::string & model_file_name);

    void save_model(const std::string & model_file_name);

    double score_doc(Xapian::FeatureVector fv);
    
    //vector<double> listmle_train(vector<instance> & instances, double tolerance_rate, double learning_rate);
    
    vector<double> log_reg_train(vector<Xapian::RankList> & samples);

};

}
#endif /* LISTMLE_H */
