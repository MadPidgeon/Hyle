#include <iostream>
#include <string>
#include <utility>
#include <cassert>
#include "lib.h"


// Stub implementations to satisfy the linker
pair<int,int> calcmove_o(const Board&){return {-1,-1};}
int calcmove_c(const Board&,int){return -1;}

static int8_t bagpeek(const Board &bd){
	int bag[SIZE*SIZE],p=0;
	for(int8_t clr=0;clr<SIZE;clr++){
		int n=bd.bagleft(clr);
		for(int i=0;i<n;i++)bag[p++]=clr;
	}
	if(p==0)return -1;
	return bag[rand()%p];
}

static pair<bool,pair<int,int>> parse2ints(const string &line){
	if(line.size()<2)return {false,{0,0}};
	char *endp;
	int first=strtol(line.data(),&endp,10);
	if(*endp!=' ')return {false,{0,0}};
	char *p2=endp+1;
	int second=strtol(p2,&endp,10);
	if(*p2=='\0'||*endp!='\0')return {false,{0,0}};
	return {true,{first,second}};
}

int main(int argc,char **argv){
	if(argc!=3){
		cerr<<"Usage: "<<argv[0]<<" <p1name> <p2name>"<<endl;
		return 1;
	}
	string p1name=argv[1],p2name=argv[2];

	cout<<"writeln 1 c"<<endl;
	cout<<"writeln 2 o"<<endl;

	Board bd;

	string line;
	int turn=1;
	while(!bd.full()){
		if(turn==1){
			int8_t clr=bagpeek(bd);
			assert(clr!=-1);
			cout<<"writeln 1 "<<(int)clr<<endl;
			cout<<"readln 1"<<endl;
			getline(cin,line);
			pair<bool,pair<int,int>> mres=parse2ints(line);
			if(!mres.first){
				cout<<"writelog Player 1 sent an invalid line: '"
				    <<line<<"'"<<endl;
				cout<<"invalid 1"<<endl;
				return 0;
			}
			int pos=mres.second.first;
			int clr2=mres.second.second;
			if(clr2!=(int)clr||!bd.valid_c(pos)){
				cout<<"writelog Player 1 sent an invalid move: '"
					<<line<<"'"<<endl;
				cout<<"invalid 1"<<endl;
				return 0;
			}
			bd.apply_c(pos,clr);

			if(!bd.full())cout<<"writeln 2 "<<pos<<" "<<(int)clr<<endl;
		} else {
			cout<<"readln 2"<<endl;
			getline(cin,line);
			pair<bool,pair<int,int>> mres=parse2ints(line);
			if(!mres.first){
				cout<<"writelog Player 2 sent an invalid line: '"
				    <<line<<"'"<<endl;
				cout<<"invalid 2"<<endl;
				return 0;
			}
			int idx1=mres.second.first,idx2=mres.second.second;
			if(!bd.valid_o(idx1,idx2)){
				cout<<"writelog Player 2 sent an invalid move: '"
				    <<line<<"'"<<endl;
				cout<<"invalid 2"<<endl;
				return 0;
			}
			bd.apply_o(idx1,idx2);

			cout<<"writeln 1 "<<idx1<<" "<<idx2<<endl;

			cerr<<'|';
		}

		turn=3-turn;
	}

	cout<<"timings"<<endl;
	double p1time,p2time;
	cin>>p1time>>p2time;

	cout<<"writelog TIMING: "<<p1time<<" "<<p2time<<endl;

	int score=bd.score();
	cout<<"results "<<-score<<" "<<score<<endl;
}
