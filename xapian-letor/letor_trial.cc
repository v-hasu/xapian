#include <xapian.h>
#include <xapian/intrusive_ptr.h>           
#include <xapian/types.h>
#include <xapian/visibility.h>

#include "ranker.h"
#include "ranklist.h"
#include "log_reg_rank.h"
//#include "evalmetric.h"
//#include "listmle.h"

#include "str.h"
#include "stringutils.h"
#include <string.h>

#include <list>
#include <string>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <math.h>

using namespace std;
using namespace Xapian;

std::string filename="train_data";

int main()
{
	ifstream file;
	int label,pos,i,pos2,j;
	double x;
	const char *ch,*path="/home/karthik/Documents/xapian-master/xapian-letor/mslr_dataset/Fold4/train.txt";
	file.open(path,ios::in);
	std::string str,sbstr;
	char queryid[10],abc[10];
	strcpy(queryid,"abc");
	strcpy(abc,"abc");	
	vector<Xapian::RankList> rlistvc;
	Xapian::RankList rlist;
	map<int,double> fvals;
	//Xapian::FeatureVector fv;
	while(!file.eof())
	{
		getline(file,str);
		//file>>str;		
		//cout<<"xx\n"<<str;
		if(str.empty())
		break;
		//label=1;
		if(str[0]=='0' || str[0]=='1' || str[0]=='2')
			label=0;
		else
			label=1;
		str.erase(0,2);
		pos=str.find(" ");
		sbstr=str.substr(0,pos);
		sbstr.erase(0,4);
		//rlist.set_qid(sbstr);
		if(strcmp(queryid,abc)==0) {
			//cout<<"\n2";
			strcpy(queryid,sbstr.c_str());
			//cout<<queryid;
			}
		if(strcmp(queryid,sbstr.c_str())!=0) {
			//cout<<"\n1mnb";
			rlistvc.push_back(rlist);
			rlist=RankList();	
			rlist.set_qid(sbstr);
			strcpy(queryid,sbstr.c_str());
			}
		sbstr.clear();
		str.erase(0,pos+1);
		for(i=0;i<19;i++)
		{
				pos=str.find(" ");
				sbstr=str.substr(0,pos);
				pos2=sbstr.find(":");
				sbstr.erase(0,pos2+1);
				//ch=sbstr.c_str();
				fvals.insert(pair<int,double>(i+1,atof(sbstr.c_str())));
				//cout<<atof(sbstr.c_str())<<" ";				
				sbstr.clear();
				str.erase(0,pos+1);
		}
		//str.erase(0,9);
		Xapian::FeatureVector fv;
		//fv.set_did(atoi(str.c_str()));
		//cout<<" asda "<<str<<"\n";
		fv.set_label(label);
		fv.set_fvals(fvals);
		rlist.add_feature_vector(fv);
		fvals.clear();
		//cout<<fvals[0];			
		str.clear();
		
	}
	rlistvc.push_back(rlist);
	Xapian::Log_Reg lrg;	
	lrg.set_training_data(rlistvc);	
	const std::string fl="model";
	lrg.learn_model();	
	lrg.save_model(fl);
	return 0;
}
