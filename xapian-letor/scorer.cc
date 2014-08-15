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

std::vector<double>
Scorer::get_labels(Xapian::RankList rl){

	std::vector<Xapian::FeatureVector> fvv = rl.get_fvv();
	int fvvsize = fvv.size();
	std::vector<double> labels;
	
	for (int i = 0; i <fvvsize; ++i){
		double label = fvv[i].get_label();
		if (2.0 == label){
			labels.push_back(3.0);
		} else{
			labels.push_back(label);
		}
		// labels.push_back(fvv[i].get_label());
	}

	return labels;

}

double 
Scorer::score(const Xapian::RankList & rl){
}
