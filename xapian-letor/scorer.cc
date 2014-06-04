#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <scorer.h>
#include <ranklist.h>

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

using namespace Xapian;

Scorer::Scorer() {
}

static std::vector<double>
get_labels(Xapian::RankList rl){

	std::vector<Xapian::FeatureVector> fvv = rl.get_data();
	int fvvsize = fvv.size();
	std::vector<double> labels;
	
	for (int i = 0; i <fvvsize; ++i){
		labels.push_back(fvv[i].get_label());
		std::cout<<labels[i]<<endl;
	}

	return labels;

}

static double 
get_dcg(std::vector<double> labels){

	int length = labels.size();
	double dcg = 0.0;//need error processing

	if (labels.empty()!=1){
		dcg = labels[0];
	}
	else{
		std::cout<<"label in labels is empty!"<<endl;
	}	

	for (int i = 1; i <length; ++i){
		dcg += labels[i]/(log(i+1)/log(2));
	}
	
	return dcg;

}

/*
test the ndcg_scorer() use the data from http://en.wikipedia.org/wiki/Discounted_cumulative_gain
input:[3,2,3,0,1,2]
output:0.932
*/
double
Scorer::ndcg_scorer(Xapian::RankList & rl){

	std::vector<double> labels = get_labels(rl);

	//DCG score of original ranking
	double DCG = get_dcg(labels);

	//DCG score of ideal ranking
	sort(labels.begin(),labels.begin()+labels.size(),std::greater<int>());
	double iDCG = get_dcg(labels);

	return DCG/iDCG;

}

/*
test the err_scorer() use the data from http://lingpipe-blog.com/2010/03/09/chapelle-metzler-zhang-grinspan-2009-expected-reciprocal-rank-for-graded-relevance/
input:[3,2,4]
output:0.63
*/
double
Scorer::err_scorer(Xapian::RankList & rl){

	//hard code for the a five-point scale, the 16 means 2^(5-1)
	int MAX_LABEL = 16; 

	std::vector<double> labels = get_labels(rl);
	int length = labels.size();

	//compute the satisfaction probability for lable of each doc in the ranking
	for (int i = 0; i <length; ++i){
		labels[i] = (pow(2,labels[i])-1)/MAX_LABEL;
	}

	double err_score = labels[0];

	//compute the accumulated probability for each doc which user will stop at
	for (int i = 1; i <length; ++i){

		//single stop probability
		double temp_err = (1.0/(i+1.0))*labels[i];

		//for user 
		for (int j=i-1; j>=0; --j){
			temp_err *= (1-labels[j]);
		}
		err_score += temp_err;
	}
	return err_score;

}