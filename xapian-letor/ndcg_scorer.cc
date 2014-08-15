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

static vector<double> 
get_dcg(const std::vector<double> labels, int topN){

	int labels_size = labels.size();
	vector<double> dcgs;

	dcgs.push_back(labels[0]);

	for (double position = 1; position < topN; ++position){
		double current_score;

		if (position < labels_size){
			current_score = labels[position];
		} else {
			current_score = 0;
		}

		if (1 == position){
			dcgs.push_back(dcgs[0] + current_score);
		} else {
			dcgs.push_back(dcgs[position - 1] + (current_score * log(2.0) / log(position + 1.0)));
		}

	}
	return dcgs;
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
	int TopN = labels_size;

	//DCG score of original ranking
	vector<double> DCG = get_dcg(labels, TopN);

	//DCG score of ideal ranking
	sort(labels.begin(),labels.begin()+labels.size(),std::greater<int>());

	vector<double> iDCG = get_dcg(labels, TopN);

	if (0 == iDCG[TopN-1]){
		return 0;
	} else{
		return DCG[TopN-1]/iDCG[TopN-1];
	}
}
