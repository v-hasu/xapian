#ifndef ADARANK_H
#define ADARANK_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT AdaRankRanker: public Ranker {

    vector<double> parameters;
    vector< double > queryWeights;
    vector< vector< double > > weakRankerMatrix;
    vector< pair< int, double > > weakRankerWeights;

  public:
    AdaRankRanker();

    AdaRankRanker(int metric_type);

    double getMeasure(Xapian::RankList ranklist);

    void initialize(vector<Xapian::RankList> ranklists, int feature_len, int ranklist_len);

    void weakranker(int feature_len);

    void reweightQuery();

    void batchLearning(vector<Xapian::RankList> ranklists, int feature_len, int ranklist_len);

    void train_model();

    void save_model_to_file();

    void load_model_from_file(const char *parameters_file);

    Xapian::RankList rank(Xapian::RankList & rl);

};

}
#endif /* ADARANK_H */
