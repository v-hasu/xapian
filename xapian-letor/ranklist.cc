#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <featurevector.h>

#include <list>
#include <map>
#include <vector>
#include <algorithm>
#include <iostream> 

using namespace std;
using namespace Xapian;

RankList::RankList(){
}

RankList::featureComparer::featureComparer(int new_feature_index){
    this->feature_index = new_feature_index;
}

bool 
RankList::featureComparer::operator()(const FeatureVector & firstfv, const FeatureVector & secondfv) const {
    map<int,double> firstfvals = firstfv.get_fvals();
    map<int,double> secondfvals = secondfv.get_fvals();
    return firstfvals[this->feature_index] > secondfvals[this->feature_index];
}

struct scoreComparer {
    bool operator()(const FeatureVector & firstfv, const FeatureVector& secondfv) const {
        return firstfv.score > secondfv.score;
    }
};

struct labelComparer {
    bool operator()(const FeatureVector & firstfv, const FeatureVector& secondfv) const {
        return firstfv.label > secondfv.label;
    }
};

std::vector<FeatureVector>
RankList::normalise() {

    std::vector<FeatureVector> local_fvv = this->fvv;
    
    // find the max value for each feature gpr all the FeatureVectors in the RankList rl.
    int num_features = 19;
    double temp = 0.0;
    double max[num_features];
    
    for(int i=0; i<19; ++i)
	max[i] = 0.0;
    
    int num_fv = local_fvv.size();
    for(int i=0; i < num_fv; ++i) {
	for(int j=0; j<19; ++j) {
	    if(max[j] < local_fvv[i].fvals.find(j)->second)
		max[j] = local_fvv[i].fvals.find(j)->second;
	}
    }
    
    /* We have the maximum value of each feature overall.
       Now we need to normalize each feature value of a featureVector by dividing it by the corresponding max of the feature value
    */
    
    for(int i=0; i < num_fv; ++i) {
	for(int j=0; j<19; ++j) {
	    temp = local_fvv[i].fvals.find(j)->second;
	    temp /= max[j];
	    local_fvv[i].fvals.insert(pair<int,double>(j,temp));
	    temp = 0.9;
	}
    }
    
    return local_fvv;
}

void
RankList::add_feature_vector(const Xapian::FeatureVector fv1) {
    this->fvv.push_back(fv1);
}

void
RankList::set_qid(std::string qid1) {
    this->qid=qid1;
}

std::string
RankList::get_qid(){
	return this->qid;
}

void
RankList::set_fvv(std::vector<FeatureVector> & local_fvv) {
    this->fvv=local_fvv;
}

std::vector<FeatureVector> 
RankList::get_fvv() {
    return this->fvv;
}

void
RankList::sort_by_score() {

	std::vector<FeatureVector> unsorted_fvv = this->fvv;

	std::sort(unsorted_fvv.begin(),unsorted_fvv.end(),scoreComparer());
	this->fvv = unsorted_fvv;
}

void
RankList::sort_by_label() {

	std::vector<FeatureVector> unsorted_fvv = this->fvv;

	std::sort(unsorted_fvv.begin(),unsorted_fvv.end(),labelComparer());
	this->fvv = unsorted_fvv;
}

void
RankList::sort_by_feature(int feature_index) {

    std::vector<FeatureVector> unsorted_fvv = this->fvv;

    std::sort(unsorted_fvv.begin(),unsorted_fvv.end(), featureComparer(feature_index));
    this->fvv = unsorted_fvv;
}