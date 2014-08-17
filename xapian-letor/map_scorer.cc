/* map_scorer.cc The MAP metric.
 * 
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

#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include <scorer.h>
#include <map_scorer.h>
#include <ranklist.h>

#include <iostream>

using namespace std;

using namespace Xapian;

MAPScorer::MAPScorer() {
}

double
MAPScorer::score(const Xapian::RankList & rl){

	std::vector<double> labels = get_labels(rl);
	double length = (double)labels.size();
	double num_of_relevant = 0.0;
	double avgPrecision = 0.0;

	for (double i = 0.0; i <length; ++i){
		if (labels[i] > 0.0){
			num_of_relevant ++;
			avgPrecision += num_of_relevant/(i+1.0);
		}
	}

	if (num_of_relevant > 0.0){
		return (avgPrecision / num_of_relevant);
	} else {
		return 0.0;
	}
}