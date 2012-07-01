/* listmle.h: The abstract ranker file.
 *
 * Copyright (C) 2012 Rishabh Mehrotra
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301
 * USA
 */

#ifndef LISTMLE_H
#define LISTMLE_H


#include <xapian.h>
#include <xapian/base.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
//#include "evalmetric.h"

#include <list>
#include <map>
#include <vector>

using namespace std;


namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT ListMLE: public Ranker {

    string model = null;
    double[] weight;
    vector<double> params;
    double tolerance_rate;
    double learning_rate;
    
  public:
    ListMLE() {};

    /* Override all the four methods below in the ranker sub-classes files
     * wiz listmle.cc , listnet.cc, listmle.cc and so on
     */
    Xapian::RankList rank(const Xapian::RankList & rl);

    void learn_model();

    void load_model(const std::string & model_file);

    void save_model();

    double score(const Xapian::FeatureVector & fv);

};

}
#endif /* LISTMLE_H */
