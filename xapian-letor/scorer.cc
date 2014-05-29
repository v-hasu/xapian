#include <xapian.h>
#include <xapian/intrusive_ptr.h>           //#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <scorer.h>
#include <ranklist.h>

#include <math.h>

#include <algorithm>

using namespace std;

using namespace Xapian;

Scorer::Scorer() {
}

static double get_dcg(std::vector<double> lables){

	int test[6]={3,2,3,0,1,2};//test code
	double dcg = test[0];
	int length = lables.size();
	
	for (int i = 1; i <6; ++i){
		dcg += (pow(test[i],2)-1)/(log(i+1)/log(2));
	}
	
	return dcg;

}

double
ndcg_scorer(Xapian::RankList & rl){//NGCD first

	std::vector<Xapian::FeatureVector> fvv = rl.get_data();
	int fvvsize = fvv.size();
	std::vector<double> lables;

	for (int i = 0; i <fvvsize; ++i){
		lables[i] = fvv[i].get_label();
	}

	double DCG = get_dcg(lables);
	sort(lables.begin(),lables.begin()+fvvsize);
	double iDCG = get_dcg(lables);

	return DCG/iDCG;

}

double
err_scorer(Xapian::RankList & rl){
	//need to be test

	int MAX_LABEL = 16; //hard code first, meaning 2^(5-1)

	std::vector<Xapian::FeatureVector> fvv = rl.get_data();
	int fvvsize = fvv.size();
	std::vector<double> lables;

	for (int i = 0; i <fvvsize; ++i){
		lables[i] = pow(2,fvv[i].get_label()-1)/MAX_LABEL;
	}

	double err_score = 0;

	for (int i = 0; i <fvvsize; ++i){
		double temp_err = 1/(i+1)*lables[i];

		for (int j=i; i>0; --i)
			temp_err *= (1-lables[j]);

		err_score += temp_err;
	}

	return err_score;

}



