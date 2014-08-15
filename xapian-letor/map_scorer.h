#ifndef MAPSCORER_H
#define MAPSCORER_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <ranklist.h>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT MAPScorer: public Scorer {

  public: 
	MAPScorer();

	double score(const Xapian::RankList & rl);

};

}

#endif /* MAPSCORER_H */
