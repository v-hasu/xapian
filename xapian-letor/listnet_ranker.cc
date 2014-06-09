#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "listnet_ranker.h"

using namespace std;
using namespace Xapian;

ListNET::ListNET(){
}

void 
ListNET::train_model(){

}

void 
ListNET::save_model(){

}

void 
ListNET::load_model(const std::string & model_file){

}

Xapian::RankList 
ListNET::rank(Xapian::RankList & rl){

}
