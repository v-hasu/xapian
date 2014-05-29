#ifndef SCORER_H
#define SCORER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>           //#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <ranklist.h>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Scorer {

  public: 
	Scorer();

	double ndcg_scorer(RankList rl);

};

}

#endif /* SCORER_H */
