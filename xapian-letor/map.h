#ifndef MAP_H
#define MAP_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"

#include <list>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <fstream>
#include <math.h>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT MAP {

	/*public:
		int num_of_results;
		double score;*/

	public:
		double ave_precision(Xapian::RankList rlist,int k);

		double map_score(vector<Xapian::RankList> rlistvc);
		
};

}

#endif /* MAP_H */
		
