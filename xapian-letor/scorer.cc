#include <xapian.h>
#include <xapian/intrusive_ptr.h>           //#include <xapian/base.h>
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

static double get_dcg(std::vector<double> lables){

	//int test[6]={3,2,3,0,1,2};//test code
	int length = lables.size();

	double dcg = 0.0;//need error processing
	if (lables.empty()!=1)
		 dcg = lables[0];//test[0];
	else
		std::cout<<"lables in scorer is empty!"<<endl;
	
	for (int i = 1; i <length; ++i){
		dcg += lables[i]/(log(i+1)/log(2));
	}
	
	return dcg;

}

double
Scorer::ndcg_scorer(Xapian::RankList & rl){//NGCD first

	std::vector<Xapian::FeatureVector> fvv = rl.get_data();
	int fvvsize = fvv.size();
	std::vector<double> lables;
	/*
	 *test date from http://en.wikipedia.org/wiki/Discounted_cumulative_gain
	lables.push_back(3.0);
	lables.push_back(2.0);
	lables.push_back(3.0);
	lables.push_back(0.0);
	lables.push_back(1.0);
	lables.push_back(2.0);
	*/
	
	for (int i = 0; i <fvvsize; ++i){
		lables.push_back(fvv[i].get_label());
	}

	double DCG = get_dcg(lables);
	sort(lables.begin(),lables.begin()+fvvsize,std::greater<int>());
	double iDCG = get_dcg(lables);

	std::cout<<"DCG"<<DCG<<endl;
	std::cout<<"iDCG"<<iDCG<<endl;
	std::cout<<"ndcg"<<DCG/iDCG<<endl;
	return DCG/iDCG;

}

double
Scorer::err_scorer(Xapian::RankList & rl){
	//need to be test
	int MAX_LABEL = 16; //hard code first, meaning 2^(5-1)

	std::vector<Xapian::FeatureVector> fvv = rl.get_data();
	int fvvsize = 3;//fvv.size();
	std::vector<double> lables;

	// *test date from http://en.wikipedia.org/wiki/Discounted_cumulative_gain
	lables.push_back(3.0);
	lables.push_back(2.0);
	lables.push_back(4.0);

	for (int i = 0; i <fvvsize; ++i){
		lables[i] = (pow(2,lables[i])-1)/MAX_LABEL;//fvv[i].get_label()
		std::cout<<"lables:"<<lables[i]<<endl;
	}

	double err_score = lables[0];
	std::cout<<"ERR:"<<err_score<<endl;

	for (int i = 1; i <fvvsize; ++i){
		double temp_err = (1.0/(i+1.0))*lables[i];
		std::cout<<"lables:"<<lables[i]<<endl;
		std::cout<<"temp_err:"<<temp_err<<endl;
		for (int j=i-1; j>=0; --j)
			temp_err *= (1-lables[j]);
		std::cout<<"temp_err:"<<temp_err<<endl;
		err_score += temp_err;
		std::cout<<"ERR:"<<err_score<<endl;
	}
	std::cout<<"ERR:"<<err_score<<endl;
	return err_score;

}



