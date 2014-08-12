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
		labels.push_back(fvv[i].get_label());
	}

	return labels;

}

double 
Scorer::score(const Xapian::RankList & rl){
}
