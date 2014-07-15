#ifndef ERRSCORER_H
#define ERRSCORER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>           //#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <ranklist.h>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT ERRScorer: public Scorer {

  public: 
	ERRScorer();

	double score(const Xapian::RankList & rl);

};

}

#endif /* ERRSCORER_H */
