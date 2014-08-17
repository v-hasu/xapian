/* scorer.cc The abstract evaluation score file.
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
#include <ranklist.h>

#include <math.h>
#include <algorithm>
#include <iostream>

using namespace std;

using namespace Xapian;

Scorer::Scorer() {
}

std::vector<double>
Scorer::get_labels(Xapian::RankList rl){

	std::vector<Xapian::FeatureVector> fvv = rl.get_fvv();
	int fvvsize = fvv.size();
	std::vector<double> labels;
	
	for (int i = 0; i <fvvsize; ++i){
		double label = fvv[i].get_label();
		if (2.0 == label){
			labels.push_back(3.0);
		} else{
			labels.push_back(label);
		}
		// labels.push_back(fvv[i].get_label());
	}

	return labels;

}

double 
Scorer::score(const Xapian::RankList & rl){
}
