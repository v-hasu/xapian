/** @file letor_evaluation.cc
 * @brief Command line ranking tool using Xapian::Letor
 */
/* Copyright (C) 2004,2005,2006,2007,2008,2009,2010 Olly Betts
 * Copyright (C) 2011 Parth Gupta
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

#include <cstdlib>
#include <cstring>
#include <stdlib.h>

#include <xapian.h>
#include <xapian/letor.h>

#include <iostream>
#include <string>

#include "gnu_getopt.h"

using namespace std;

#define PROG_NAME "letor_evaluation"
#define PROG_DESC "Xapian Lerning to Rank Module Evaluation"

static void show_usage() {
    cout << "Usage: "PROG_NAME"\n"
"Options:\n"
"  -a, --training_set=TRAINING_SET_PATH  training_set path\n"
"  -e, --test_set=TEST_SET_PATH  test_set path\n"
"  -r, --ranker=RANKER  specify the ranking algorithm\n"
"                       Supported algorithm: (default=ListNet)\n"
"                       0:svmrank\n"
"                       1:ListNet\n"
"                       2:ListMLE\n"
"                       3:Adarank\n"
"                       4:borda-fuse (ranking aggregation)\n"
"  -e, --metric=METRIC  specify the metric to evaluate the ranking result\n"
"                       Supported metric: MAP, NGCG, ERR(default=MAP)\n"
"                       0:MAP\n"
"                       1:NDCG\n"
"                       2:ERR\n"
"  -i, --iterations=ITERATIONS  The number of iterations(default=25)\n"
"  -l, --learning_rate=LEARNING_RATE  learning_rate(default=0.01)\n"
"  -h, --help          	display this help and exit\n"
"  -v, --version       	output version information and exit\n";
}

static void show_options(string training_set_path, string test_set_path, int ranker_type, int metric_type, int iterations, double learning_rate) {
	cout << endl;
    cout << "Options:" << endl;
    cout << "training_set_path: " << training_set_path << endl;
    cout << "test_set_path: " << test_set_path << endl;

    if (0 == ranker_type){
    	cout << "ranker_type: svm-rank" << endl;
    } else if (1 == ranker_type) {
    	cout << "ranker_type: ListNet" << endl;
    } else if (2 == ranker_type) {
    	cout << "ranker_type: ListMLE" << endl;
    } else if (3 == ranker_type) {
    	cout << "ranker_type: AdaRank" << endl;
    } else if (4 == ranker_type) {
    	cout << "ranker_type: Hybird model" << endl;
    }
    
    if (0 == metric_type){
    	cout << "metric_type: MAP" << endl;
    } else if (1 == ranker_type) {
    	cout << "metric_type: NDCG" << endl;
    } else if (2 == ranker_type) {
    	cout << "metric_type: ERR" << endl;
    }

    if (0 != ranker_type) {
    	cout << "iterations: " << iterations << endl;
    }

    if (3 != ranker_type){
    	cout << "learning_rate: " << learning_rate << endl;
    }
    cout << endl;
}

int
main(int argc, char **argv){

	const char * opts = "a:e:r:m:i:l:h:v";
    static const struct option long_opts[] = {
	{ "training_set",	required_argument, 0, 'a' },
	{ "test_set",	required_argument, 0, 'e' },
    { "ranker",	optional_argument, 0, 'r' },
    { "metric",	optional_argument, 0, 'm' },
    { "iterations",	optional_argument, 0, 'i' },
    { "learning_rate",	optional_argument, 0, 'l' },
	{ "help",	no_argument, 0, 'h' },
	{ "version",	no_argument, 0, 'v' },
	{ NULL,		0, 0, 0}
    };

    string training_set_path;
    string test_set_path;
    int ranker_type = 1;
    int metric_type = 0;
    int iterations = 25;
    double learning_rate = 0.01;

    int c;
    while ((c = gnu_getopt_long(argc, argv, opts, long_opts, 0)) != -1) {
		switch (c) {

			case 'a':
	        training_set_path.assign(optarg);
	        break;

	        case 'e':
	        test_set_path.assign(optarg);
	        break;

			case 'r':
	        ranker_type = atoi(optarg);
	        if (0 > ranker_type || 4 < ranker_type){
	        	cout << "ranker_type out of range!\nborda-fuse temporarily can't be using" << endl;
	        	show_usage();
				exit(1);
	        }
	        break;

		    case 'm':
			metric_type = atoi(optarg);
			if (0 > metric_type || 2 < metric_type){
	        	cout << "metric_type out of range!" << endl;
	        	show_usage();
				exit(1);
	        }
			break;

		    case 'i':
			iterations = atoi(optarg);
			break;

			case 'l':
			learning_rate = atof(optarg);
			break;
		    
		    case 'v':
			cout << PROG_NAME" - "PROG_DESC << endl;
			exit(0);
		    case 'h':
			cout << PROG_NAME" - "PROG_DESC"\n\n";
			show_usage();
			exit(0);
		    case ':': // missing parameter
		    case '?': // unknown option
			show_usage();
			exit(1);
		}
    }

    show_options(training_set_path, test_set_path, ranker_type, metric_type, iterations, learning_rate);

	Xapian::Letor ltr;
    ltr.create_ranker(ranker_type,metric_type,iterations,learning_rate);
    ltr.set_training_set(training_set_path);
    ltr.set_test_set(test_set_path);
    ltr.letor_learn_model();
    ltr.letor_rank();

}
