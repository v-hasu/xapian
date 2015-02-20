
#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"
#include "log_reg_rank.h"

#include "str.h"
#include "stringutils.h"
#include <string.h>

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <math.h>

using namespace std;
using namespace Xapian;

void
Log_Reg::set_training_data(vector<Xapian::RankList> training_data1) {
    this->training_data = training_data1;
}

void 
Log_Reg::learn_model() {
	//const std::string model_file_name="model";
	Log_Reg::parameters=Log_Reg::log_reg_train(this->training_data);
	//Log_Reg::save_model(model_file_name);
}

vector<double>
Log_Reg::log_reg_train(vector<Xapian::RankList> & samples) {
	unsigned int sample_len=samples.size();
	Xapian::RankList sample_data;
	vector<Xapian::FeatureVector> fv;
	vector<double> theta;
	
	unsigned int i,j,k,ep=0;
	for(i=0;i<19;i++)
		theta.push_back(((double)1)/(i+1));

	/*cout<<"\n";
	for(i=0;i<19;i++)
		cout<<theta[i]<<" ";*/

	while(ep<this->number_of_epochs) {
		for(i=0;i<sample_len;i++) {
			sample_data=samples[i];
			fv=sample_data.get_data();
			
			for(j=0;j<fv.size();j++) {
				//cout<<ep<<"   "<<i<<"\n";
				double dotprod=1;
				map<int,double> fvals=fv[j].get_fvals();
				map<int,double>::const_iterator iter;
				 
				for(iter=fvals.begin();iter!=fvals.end();iter++) {
					dotprod=dotprod+fvals[iter->second]*theta[iter->first-1];
					}
				
				dotprod=1/(1+exp(-1*dotprod));
				
				for(iter=fvals.begin();iter!=fvals.end();iter++) {
					theta[iter->first-1]=theta[iter->first-1]+this->learning_rate*(fv[j].get_label()-dotprod)*fvals[iter->second];
					}
			
						
				}
			fv.clear();
			}
		ep=ep+1;
		}
	
	return theta;

}				
				
void 
Log_Reg::load_model(const std::string & model_file_name) {
	ifstream model_file;
    model_file.open(model_file_name.c_str(),ios::in);
    string str;
    while(!model_file.eof()) {
	getline(model_file,str);
	if (str.empty())
	    break;
	parameters.push_back(atof(str.c_str())); 
	str.clear();
    }
    model_file.close();
}

void 
Log_Reg::save_model(const std::string & model_file_name) {
	ofstream train_file;
    train_file.open(model_file_name.c_str());
    int num_param=parameters.size();

    for (int i=0; i<num_param; ++i)
	train_file << parameters[i]<<"\n";
    
    train_file.close();
}

double 
Log_Reg::score_doc(Xapian::FeatureVector fv) {
	double score=0.0,lb,original_lb;
    int num_param=parameters.size();
    map<int,double> local_fvals = fv.get_fvals();
    
    for(int i=1; i<=num_param; ++i)
	score += (parameters[i]*local_fvals.find(i)->second);
    
	score=1/(1+exp(-1*score));

	if(score>0.5)
		lb=1;
	else
		lb=0;
	
	original_lb=fv.get_label();

	if(lb==original_lb)
		return 1.0;

	else
		return 0.0;    
		
	return score;
}
					
					
			
			
		
		

