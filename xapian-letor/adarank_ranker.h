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

    int iterations;
    vector< double > queryWeights;  //record the weight of each query
    vector< vector< double > > weakRankerMatrix;    //ranker performance matrix
    vector< vector< double > > perfPerModel;    //record the model's performance on each query
    vector< pair< int, double > > weakRankerWeights;    //the linear combination of weakers of rankers

  public:
    AdaRankRanker();

    AdaRankRanker(int metric_type);

    AdaRankRanker(int metric_type, int new_iterations);

    double getMeasure(RankList ranklist, int feature_index);

    void initialize(vector<RankList> ranklists, int feature_len, int ranklist_len);

    void createWeakranker(int feature_len);

    void reweightQuery(int query_num);

    void batchLearning(vector<RankList> ranklists, int feature_len, int ranklist_len);

    void train_model();

    void save_model_to_file();

    void load_model_from_file(const char *parameters_file);

    Xapian::RankList rank(RankList & rl);

};

}
#endif /* ADARANK_H */
