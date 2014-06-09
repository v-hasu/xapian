/* ranker.cc: The abstract ranker file.
 *
 * Copyright (C) 2012 Parth Gupta
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
 
#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranklist.h"
#include "ranker.h"

#include <vector>

using namespace std;
using namespace Xapian;


Ranker::Ranker() {

}

std::vector<Xapian::RankList> 
Ranker::get_traindata(){
    return this->traindata;
}

void
Ranker::set_training_data(vector<Xapian::RankList> training_data1) {
    this->traindata = training_data1;
}

Xapian::Scorer 
Ranker::get_scorer(){
    return this->scorer;
}

void
Ranker::train_model() {
}

void
Ranker::save_model() {
}

void
Ranker::load_model(const std::string & model_file) {
}

Xapian::RankList
Ranker::rank(Xapian::RankList & rl) {
}