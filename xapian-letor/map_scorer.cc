#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <scorer.h>
#include <map_scorer.h>
#include <ranklist.h>

#include <iostream>

using namespace std;

using namespace Xapian;

MAPScorer::MAPScorer() {
}

double
MAPScorer::score(const Xapian::RankList & rl){

	std::vector<double> labels = get_labels(rl);
	double length = (double)labels.size();
	double num_of_relevant = 0.0;
	double avgPrecision = 0.0;

	for (double i = 0.0; i <length; ++i){
		if (labels[i] > 0.0){
			num_of_relevant ++;
			avgPrecision += num_of_relevant/(i+1);
		}
	}

	if (num_of_relevant > 0.0){
		return (avgPrecision / num_of_relevant);
	} else {
		return 0.0;
	}
}