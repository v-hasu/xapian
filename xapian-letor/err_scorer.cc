#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <scorer.h>
#include <err_scorer.h>
#include <ranklist.h>

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

using namespace Xapian;

ERRScorer::ERRScorer() {
}


/*
test the err_scorer() use the data from http://lingpipe-blog.com/2010/03/09/chapelle-metzler-zhang-grinspan-2009-expected-reciprocal-rank-for-graded-relevance/
input:[3,2,4]
output:0.63
*/
double
ERRScorer::score(const Xapian::RankList & rl){

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