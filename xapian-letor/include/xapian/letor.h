/** @file letor.h
 *  @brief weighting scheme based on Learning to Rank. Note: letor.h is not a part of official stable Xapian API.
 */
/* Copyright (C) 2011 Parth Gupta
 * Copyright (C) 2014 Hanxiao Sun
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301 USA
 */

#ifndef XAPIAN_INCLUDED_LETOR_H
#define XAPIAN_INCLUDED_LETOR_H

#include <xapian.h>
#include <xapian/intrusive_ptr.h>
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <string>
#include <map>

using namespace std;

namespace Xapian {

class XAPIAN_VISIBILITY_DEFAULT Letor {

  public:
    
    /// @private @internal Class representing the Letor internals.
    class Internal;
    /// @private @internal Reference counted internals.
    Xapian::Internal::intrusive_ptr<Internal> internal;

    /// Copy constructor.
    Letor(const Letor & o);

    /// Assignment.
    Letor & operator=(const Letor & o);

    /// Default constructor.
    Letor();

    /// Destructor.
    ~Letor();

    /// Specify the database to use for retrieval. This database will be used directly by the methods of Xapian::Letor::Internal
    void set_database(const Xapian::Database & db);

    /// Specify the query. This will be used by the internal class.
    void set_query(const Xapian::Query & query);

    void set_training_set(string tringing_set_path);

    void set_test_set(string test_set_path);

    void letor_rank();

    void letor_learn_model();

    void prepare_training_file(const std::string & query_file, const std::string & qrel_file, Xapian::doccount msetsize);
    
    void create_ranker(int ranker_type, int metric_type, int iteration, double learning_rate);
    
};

}

#endif /* XAPIAN_INCLUDED_LETOR_H */
