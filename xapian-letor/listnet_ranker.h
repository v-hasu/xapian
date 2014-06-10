#ifndef LISTNET_H
#define LISTNET_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT ListNET: public Ranker {

    vector<double> parameters;

  public:
    ListNET();

    void train_model();

    void save_model_to_file();

    void load_model_from_file(const std::string & model_file);

    Xapian::RankList rank(Xapian::RankList & rl);

};

}
#endif /* LISTMLE_H */
