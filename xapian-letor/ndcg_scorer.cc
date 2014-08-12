#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <scorer.h>
#include <ndcg_scorer.h>
#include <ranklist.h>

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

using namespace Xapian;

NDCGScorer::NDCGScorer() {
}

static double 
get_dcg(const std::vector<double> labels, int N){

	int length = labels.size();
	int dcg_size;
	if (N < length){
		dcg_size = N;
	} else{
		dcg_size = length;
	}
	
	double dcg = 0.0;//need error processing

	if (labels.empty()!=1){
		dcg = labels[0];
	}
	else{
		std::cout<<"label in labels is empty!"<<endl;
	}	

	for (int i = 1; i <dcg_size; ++i){
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
NDCGScorer::score(const Xapian::RankList & rl){

	std::vector<double> labels = get_labels(rl);
	int labels_size = labels.size();

	//DCG score of original ranking
	double DCG = get_dcg(labels, labels_size);

	//DCG score of ideal ranking
	sort(labels.begin(),labels.begin()+labels.size(),std::greater<int>());

	double iDCG = get_dcg(labels, labels_size);

	if (iDCG==0){
		return 1;
	} else{
		return DCG/iDCG;
	}
}
