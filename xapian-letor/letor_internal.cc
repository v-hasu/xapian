/** @file letor_internal.cc
 * @brief Internals of Xapian::Letor class
 */
/* Copyright (C) 2011 Parth Gupta
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

#include <xapian.h>

#include "letor_internal.h"
#include "featuremanager.h"
#include "str.h"
#include "stringutils.h"
#include "ranker.h"
#include "svm_ranker.h"
#include "letor_features.h"

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include "safeerrno.h"
#include "safeunistd.h"

#include <algorithm>
#include <list>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <math.h>

#include <stdio.h>

#include <libsvm/svm.h>
#define Malloc(type, n) (type *)malloc((n) * sizeof(type))

using namespace std;

using namespace Xapian;

typedef vector<Xapian::RankList> Samples;

int MAXPATHLEN = 200;

static string get_cwd() {
    char temp[MAXPATHLEN];
    return (getcwd(temp, MAXPATHLEN) ? std::string(temp) : std::string());
}

//Stop-words
static const char * sw[] = {
    "a", "about", "an", "and", "are", "as", "at",
    "be", "by",
    "en",
    "for", "from",
    "how",
    "i", "in", "is", "it",
    "of", "on", "or",
    "that", "the", "this", "to",
    "was", "what", "when", "where", "which", "who", "why", "will", "with"
};

static void
write_to_file(std::vector<Xapian::RankList> list_rlist) {

    /* This function will save the list<RankList> to the training file
     * so that this list<RankList> can be loaded again by train_model() and subsequent functions.
     */

    ofstream train_file;
    train_file.open("train.txt");
    // write it down with proper format
    int size_rlist = list_rlist.size();//return the size of list_rlist
    //for (list<Xapian::RankList>::iterator it = l.begin(); it != l.end(); it++);
    for(int i = 0; i < size_rlist; ++i) {
       RankList rlist = list_rlist[i];
        /* now save this RankList...each RankList has a vectorr<FeatureVector>
         * each FeatureVector has the following data: double score, int fcount, string did, map<int, double> fvals
         * each line: double int string 1:double 2:double 3:double....
         */
        int size_rl = rlist.fvv.size();
        // print the size of the rlist so that later we know how many featureVector to scan for this particular rlist.
        //train_file << size_rl << " " << rlist.qid << endl;
        string qid =rlist.get_qid();
        for(int j=0; j < size_rl; ++j) {
            FeatureVector fv = rlist.fvv[j];
            // now save this feature vector fv to the file
            //cout <<"label"<<fv.label<< "fv.score " << fv.score << "fv.fcount " << fv.fcount << "fv.did " << fv.did << endl;// << " ";
            train_file << fv.label << " qid:" <<qid;// << " ";
            if (fv.fcount==0){cout << "fcount is empty";}
            for(int k=1; k < fv.fcount; ++k) {//just start from 1
            train_file << " " << k << ":" << fv.fvals.find(k)->second;
            //cout << "write fcount" << endl;
            }
            train_file <<" #docid=" << fv.did<<endl;
        }
    }
    train_file.close();
}

/* Load the data from the MSRA letor4.0 dataset */
vector<Xapian::RankList>
Letor::Internal::load_ranklist_from_letor4(const char *filename, bool isTrain) { 
    vector<Xapian::RankList> ranklist_list;

    fstream train_file (filename, ios::in);
    if(!train_file.good()){
        if (isTrain) {
            cout << "No train file found"<<endl;
        } else {
            cout << "No test file found"<<endl;
        }
    }

    std::vector<FeatureVector> fvv;
    string lastqid = " ";//record the lastqid, if not the same the current qid, then new a ranklist

    while (train_file.peek() != EOF) {
        FeatureVector fv;//new a fv

        double label;//read label
        train_file >> label;
        fv.set_label(label);

        train_file.ignore();

        string qid;//read qid
        train_file >> qid;

        qid = qid.substr(qid.find(":")+1,qid.length());

        if (lastqid == " "){//give the current qid to the last qid, when the first time using the lastqid
            lastqid = qid;
        }

        int feature_index;
        double feature_value;
        map<int,double> fvals;//read features
        for(int i = 0; i < 46; ++i){//46 features in the letor4.0 dateset, hard code
            train_file.ignore();
            train_file >> feature_index;

            train_file.ignore();
            train_file >> feature_value;

            //the feature index is begin from 1, in order to simply the computing, let it start from 0
            fvals[feature_index-1] = feature_value; 
        }
        
        fv.set_fvals(fvals);

        string did;
        train_file.ignore(100,'=');
        train_file >> did;
        fv.set_did(did);

        train_file.ignore(100,'\n');

        fv.set_fcount(46);//hard code now! only for letor4.0
        fv.set_score(0);

        fvv.push_back(fv);

        //when the qid is not the same as the last qid, it is the time to new a ranklist
        if (qid != lastqid) {
            RankList rlist;
            rlist.set_qid(lastqid);
            rlist.set_fvv(fvv);
            ranklist_list.push_back(rlist);
            fvv = std::vector<FeatureVector>();
            lastqid = qid;
        }
        
    }   
    //push back the last ranklist
    RankList rlist;
    rlist.set_qid(lastqid);
    rlist.set_fvv(fvv);
    ranklist_list.push_back(rlist);

    train_file.close();

    if (isTrain) {
        cout << "the size of ranklist read from the training set: " << ranklist_list.size() << endl << endl;
    } else {
        cout << "the size of ranklist read from the test set: " << ranklist_list.size() << endl << endl;
    }
    
    return ranklist_list;
}

void 
Letor::Internal::letor_learn_model(){

    string training_set_file_name = get_cwd().append(this->training_set);
    
    vector<Xapian::RankList> samples = load_ranklist_from_letor4(training_set_file_name.c_str(), true);

    vector<Xapian::FeatureVector> trainfvv = samples[0].get_fvv();

    int rankers_size = rankers.size();

    if (0 == rankers_size){
        cout << "no ranker assigned!!" <<endl;
        exit(2);
    } else {
        for (int i = 0; i < rankers_size; ++i){
            rankers[i]->set_training_data(samples);
            rankers[i]->train_model();
        }
    }
    
}

void
Letor::Internal::letor_rank_from_letor4() {

    vector<string> rankeddid;
    vector<double> scores;
    string input_file_name;

    input_file_name = get_cwd().append(this->test_set);
    
    vector<Xapian::RankList> samples = load_ranklist_from_letor4(input_file_name.c_str(), false);
    
    int samples_size = samples.size();
    int rankers_size = rankers.size();

    for (int i = 0; i < samples_size; ++i){
        double Score = 0.0;

        if (1 == rankers_size){
            Xapian::RankList ranklist = rankers[0]->rank(samples[i]);
            Score = rankers[0]->get_score(ranklist);

        } else if (1 < rankers_size){

            vector<FeatureVector> fvv = samples[i].get_fvv();
            map<int,double> fvals = fvv[0].get_fvals();

            vector<RankList> rls;

            for (int j = 0; j < rankers_size; ++j){
                Xapian::RankList ranklist = rankers[j]->rank(samples[i]); 
                rls.push_back(ranklist);
            }

            Xapian::RankList combined_ranklist = rankers[0]->aggregate(rls);
            Score = rankers[0]->get_score(combined_ranklist);
            
        } else {
            cout << "no ranker assigned!!" <<endl;
            exit(2);
        }

        scores.push_back(Score);
    }
    double final_score = (accumulate(scores.begin() , scores.end() , 0.0)/(double)scores.size());
    cout << "the final scores is: " << final_score << endl;
}

void
Letor::Internal::prepare_training_file(const string & queryfile, const string & qrel_file, Xapian::doccount msetsize) {

//    ofstream train_file;
//    train_file.open("train.txt");

    Xapian::SimpleStopper mystopper(sw, sw + sizeof(sw) / sizeof(sw[0]));
    Xapian::Stem stemmer("english");

    Xapian::QueryParser parser;
    parser.add_prefix("title", "S");
    parser.add_prefix("subject", "S");

    parser.set_database(letor_db);
    parser.set_default_op(Xapian::Query::OP_OR);
    parser.set_stemmer(stemmer);
    parser.set_stemming_strategy(Xapian::QueryParser::STEM_SOME);
    parser.set_stopper(&mystopper);

    /* ---------------------------- store whole qrel file in a Map<> ---------------------*/

//    typedef map<string, int> Map1;      //docid and relevance judjement 0/1
//    typedef map<string, Map1> Map2;     // qid and map1
//    Map2 qrel;

    //map<string, map<string, int> > qrel; // 1

    Xapian::FeatureManager fm;
    fm.set_database(letor_db);
    fm.load_relevance(qrel_file);
    //qrel = fm.load_relevance(qrel_file);

    vector<Xapian::RankList> list_rlist;

    string str1;
    ifstream myfile1;
    myfile1.open(queryfile.c_str(), ios::in);

    if(!myfile1.good()){
        cout << "No Query file found"<<endl;
    }


    while (!myfile1.eof()) {           //reading all the queries line by line from the query file

        getline(myfile1, str1);

        if (str1.empty()) {
            //cout<< "str1 empty";
            break;
        }

        string qid = str1.substr(0, (int)str1.find(" "));
        string querystr = str1.substr((int)str1.find("'")+1, (str1.length() - ((int)str1.find("'") + 2)));

        string qq = querystr;
        istringstream iss(querystr);
        string title = "title:";
        while (iss) {
            string t;
            iss >> t;
            if (t.empty())
            break;
            string temp;
            temp.append(title);
            temp.append(t);
            temp.append(" ");
            temp.append(qq);
            qq = temp;
        }

        //cout << "Processing Query: " << qq << "\n";
      
        Xapian::Query query = parser.parse_query(qq,
                             parser.FLAG_DEFAULT|
                             parser.FLAG_SPELLING_CORRECTION);

        Xapian::Enquire enquire(letor_db);
        enquire.set_query(query);

        Xapian::MSet mset = enquire.get_mset(0, msetsize);

        fm.set_query(query);

        Xapian::RankList rl = fm.create_rank_list(mset, qid, 1);

        list_rlist.push_back(rl);
    }//while closed
    //cout<<"myfile1.close before"<<endl;
    myfile1.close();
    //cout<<"myfile1.close after"<<endl;
    /* Call either one of the following
     */
    write_to_file(list_rlist);
    //cout<<"write file"<<endl;
    //write_ranklist(list_rlist);
    //cout<<"write ranklist"<<endl;
//    train_file.close();
}