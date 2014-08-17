/** @file letor.cc
 * @brief Letor Class
 */
/* Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2012 Olly Betts
 * Copyright (C) 2014 Hanxiao Sun
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

#include <config.h>
#include <xapian/letor.h>
#include "letor_internal.h"
#include "ranker.h"
#include "svm_ranker.h"
#include "listnet_ranker.h"
#include "listmle_ranker.h"
#include "adarank_ranker.h"

#include <vector>
#include <map>

using namespace std;

namespace Xapian {

Letor::Letor(const Letor & o) : internal(o.internal) { }

Letor &
Letor::operator=(const Letor & o)
{
    internal = o.internal;
    return *this;
}

Letor::Letor() : internal(new Letor::Internal) { }

Letor::~Letor() { }

void
Letor::set_database(const Xapian::Database & db) {
    internal->letor_db = db;
}

void
Letor::set_query(const Xapian::Query & query) {
    internal->letor_query = query;
}

void 
Letor::set_training_set(string training_set_path) {
    internal->training_set = training_set_path;
}

void 
Letor::set_test_set(string test_set_path) {
    internal->test_set = test_set_path;
}

void
Letor::letor_rank() {
    internal->letor_rank_from_letor4();
}

void
Letor::letor_learn_model() {
    internal->letor_learn_model();
}

void
Letor::prepare_training_file(const string & query_file, const string & qrel_file, Xapian::doccount msetsize) {
    internal->prepare_training_file(query_file, qrel_file, msetsize);
}

void
Letor::create_ranker(int ranker_type, int metric_type, int iterations, double learning_rate) {
    switch(ranker_type) {
        case 0: 
                internal->rankers.push_back(new SVMRanker(metric_type));
                cout << "SVMRanker created!" <<endl;
                break;
        case 1: 
                internal->rankers.push_back(new ListNETRanker(metric_type, iterations, learning_rate));
                cout << "ListNETRanker created!" <<endl;
                break;
        case 2: 
                internal->rankers.push_back(new ListMLERanker(metric_type, iterations, learning_rate));
                cout << "ListMLERanker created!" <<endl;
                break;
        case 3: 
                internal->rankers.push_back(new AdaRankRanker(metric_type, iterations));
                cout << "AdaRankRanker created!" <<endl;
                break;
        case 4: 
                cout << "Hybird ranking model!" <<endl;
                internal->rankers.push_back(new ListNETRanker(metric_type, iterations, learning_rate));
                cout << "ListNETRanker created!" <<endl;
                internal->rankers.push_back(new ListMLERanker(metric_type, iterations, learning_rate));
                cout << "ListMLERanker created!" <<endl;
                internal->rankers.push_back(new AdaRankRanker(metric_type, iterations));
                cout << "AdaRankRanker created!" <<endl;
                break;
        default:cout<<"Please specify proper ranker.";
    }
}

}
