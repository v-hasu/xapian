#ifndef NDCGSCORER_H
#define NDCGSCORER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>           //#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <ranklist.h>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT NDCGScorer: public Scorer {

  public: 
	NDCGScorer();

	double score(const Xapian::RankList & rl);

};

}

#endif /* NDCGSCORER_H */
