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
#include "map.h"

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <math.h>

using namespace std;
using namespace Xapian;

double 
MAP::map_score(vector <Xapian::RankList> rlistvc) {
	
	Xapian::RankList rlist;	
	int rlistvc_size=rlistvc.size();
	int i;
	double score=0.0;
	for(i=0;i<rlistvc_size;i++) {
		
		rlist=rlistvc[i];
		score=score+ave_precision(rlist,20);
	
	}
	score=score/((double)rlistvc_size);
	return score;

}

double 
MAP::ave_precision(Xapian::RankList rlist,int k) {
	std::vector <Xapian::FeatureVector> fv=rlist.get_data();
	Xapian::FeatureVector fvv;	
	int num_fv=fv.size(),i;
	if(num_fv>k)
		num_fv=k;
	double correct=0.0,total=0.0,score=0.0;
	for(i=0;i<num_fv;i++) {
		
		fvv=fv[i];
		total=total+1;
		cout<<fvv.get_label()<<" ";
		if(fvv.get_label()==1.0) {
			correct=correct+1;
			score=score+(correct/total);
			
		}
		
	}
	//cout<<"score is "<<score<<" "<<total<<"\n";
	//score=score/total;
	return score;
}




















