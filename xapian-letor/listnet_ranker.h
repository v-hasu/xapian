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

class XAPIAN_VISIBILITY_DEFAULT ListNETRanker: public Ranker {

    vector<double> parameters;

    int iterations;

    double learning_rate;

  public:
    ListNETRanker();

    ListNETRanker(int metric_type);

    ListNETRanker(int metric_type, int new_iterations, double new_learning_rate);

    void train_model();

    void save_model_to_file();

    void load_model_from_file(const char *parameters_file);

    Xapian::RankList rank(Xapian::RankList & rl);

};

}
#endif /* LISTMLE_H */
