#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <algorithm>
#include <limits>
#include <climits>
#include <cstdint>
#include <cctype>
#include <cassert>
#include <ctime>
#include <sys/time.h>
#include <unistd.h>

using namespace std;

const int ORDER=0,CHAOS=1;

const int SIZE=7;


#define TIMEIT(block_) \
		do { \
			clock_t s__=clock(); \
			{block_;} \
			clock_t e__=clock(); \
			cerr<<"TIMEIT: (" #block_ ") took "<<(double)(e__-s__)/CLOCKS_PER_SEC<<'s'<<endl; \
		} while(0)


struct Move{
	int player;
	int idx,idx2,clr;
};

class Board{
public:
	int bd[SIZE*SIZE];
	
	Board(){
		memset(bd,-1,SIZE*SIZE*sizeof(int));
	}

	inline int& operator[](int idx){
		return bd[idx];
	}

	inline int operator[](int idx) const {
		return bd[idx];
	}

	bool full() const {
		int accum=0;
		for(int i=0;i<SIZE*SIZE;i++)accum|=bd[i];
		return accum>=0;
	}

	int nempty() const {
		int num=0;
		for(int i=0;i<SIZE*SIZE;i++)num+=bd[i]==-1;
		return num;
	}

	int score() const {
		int res=0;
		for(int i=0;i<SIZE;i++){
			for(int sz=2;sz<=SIZE;sz++){
				for(int j=0;j<=SIZE-sz;j++){
					int add=sz;
					int base=SIZE*i+j;
					for(int k=0;k<sz/2;k++){
						if(bd[base+k]!=bd[base+sz-1-k]){
							add=0;
							break;
						}
					}
					res+=add;

					add=sz;
					base=SIZE*j+i;
					for(int k=0;k<sz/2;k++){
						if(bd[base+k*SIZE]!=bd[base+(sz-1-k)*SIZE]){
							add=0;
							break;
						}
					}
					res+=add;
				}
			}
		}
		return res;
	}

	bool validmove(const Move &mv) const {
		if(mv.player==ORDER)return valid_o(mv.idx,mv.idx2);
		if(mv.player==CHAOS)return valid_c(mv.idx);
		return false;
	}

	bool valid_o(int idx,int idx2) const {
		return idx>=0&&idx<SIZE*SIZE&&
		       idx2>=0&&idx2<SIZE*SIZE&&
		       idx!=idx2&&
		       bd[idx]!=-1&&bd[idx2]==-1&&
		       (idx%SIZE==idx2%SIZE||idx/SIZE==idx2/SIZE);
	}

	bool valid_c(int idx) const {
		return bd[idx]==-1;
	}

	void applymove(const Move &mv){
		assert(validmove(mv));
		if(mv.player==ORDER)apply_o(mv.idx,mv.idx2);
		else if(mv.player==CHAOS)apply_c(mv.idx,mv.clr);
		else assert(false);
	}

	void apply_o(int idx,int idx2){
		bd[idx2]=bd[idx];
		bd[idx]=-1;
	}

	void apply_c(int idx,int clr){
		bd[idx]=clr;
	}

	void undo_o(int idx,int idx2){
		bd[idx]=bd[idx2];
		bd[idx2]=-1;
	}

	void undo_c(int idx){
		bd[idx]=-1;
	}

	void print(ostream &os) const {
		const char *dict[]={
			".", "\x1B[41m0\x1B[0m", "\x1B[42m1\x1B[0m", "\x1B[44m2\x1B[0m", "\x1B[43m3\x1B[0m",
			"\x1B[46m4\x1B[0m", "\x1B[45m5\x1B[0m", "\x1B[47m6\x1B[0m",
		};
		for(int y=0;y<SIZE;y++){
			for(int x=0;x<SIZE;x++){
				if(x!=0)os<<' ';
				int v=bd[SIZE*y+x];
				if(v>=-1&&v<=6)os<<dict[v+1];
				else os<<v;
			}
			os<<'\n';
		}
		os.flush();
	}
};

constexpr int max_order_moves(int n){
	return 4*((n-1)*(n-2)/2-(n-1)/2)+3*(2*(n-1));
}

#define FOR_ALL_ORDER_MOVES(bd_,idx1_,idx2_) \
		for(idx1_ = 0; idx1_ < SIZE*SIZE; idx1_++) \
			if(bd_[idx1_] != -1) \
				for(int r__ = 0, \
							x__ = idx1_%SIZE, y__ = idx1_/SIZE, dx__ = 1, dy__ = 0, t__ = 0; \
						r__ < 4; \
						r__++, \
							t__ = dx__, dx__ = -dy__, dy__ = t__) \
					if(x__ + dx__ >= 0 && x__ + dx__ < SIZE && \
							y__ + dy__ >= 0 && y__ + dy__ < SIZE && \
							(idx2_ = idx1_ + SIZE*dy__ + dx__, bd_[idx2_] == -1))

pair<int,int> randmove_o(Board &bd){
	int poss[max_order_moves(SIZE)][2],nposs=0;
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]==-1)continue;
		int x=i%SIZE,y=i/SIZE;
		int dx=1,dy=0;
		for(int r=0;r<4;r++){
			if(x+dx>=0&&x+dx<SIZE&&y+dy>=0&&y+dy<SIZE&&bd[i+SIZE*dy+dx]==-1){
				poss[nposs][0]=i;
				poss[nposs][1]=i+SIZE*dy+dx;
				nposs++;
			}

			int t=dx;
			dx=-dy;
			dy=t;
		}
	}
	if(nposs==0)return {-1,-1};
	int i=rand()%nposs;
	return {poss[i][0],poss[i][1]};
}

bool randmove_c(Board &bd){
	int poss[SIZE*SIZE],nposs=0;
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]==-1)poss[nposs++]=i;
	}
	if(nposs==0)return -1;
	return poss[rand()%nposs];
}

int playout_o(Board &bd,int left){
	while(left>0){
		pair<int,int> p=randmove_o(bd);
		bd.apply_o(p.first,p.second);

		int idx=randmove_c(bd);
		bd.apply_c(idx,rand()%SIZE);
		left--;
	}
	return bd.score();
}

int playout_o(Board &bd){
	return playout_o(bd,bd.nempty());
}

int playout_c(Board &bd){
	int poss[SIZE*SIZE];
	int left=0;
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]==-1)poss[left++]=i;
	}
	if(left==0)return -1;
	bd.apply_c(poss[rand()%left],rand()%SIZE);
	return playout_o(bd,left-1);
}

const int montecarlo_playouts=10000;

pair<int,int> calcmove_o(const Board &bd){
	int maxat[2]={-1,-1};
	int64_t maxtotal=-1;
	int idx1,idx2;
	FOR_ALL_ORDER_MOVES(bd,idx1,idx2) {
		Board bd2(bd);
		bd2.apply_o(idx1,idx2);
		int64_t total=0;
		for(int i=0;i<montecarlo_playouts;i++){
			Board bd3(bd2);
			total+=playout_o(bd3);
		}
		if(total>maxtotal){
			maxtotal=total;
			maxat[0]=idx1;
			maxat[1]=idx2;
		}
	}
	return {maxat[0],maxat[1]};
}

int calcmove_c(Board &bd,int clr){
	int minat=-1;
	int64_t mintotal=INT64_MAX;
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]!=-1)continue;
		Board bd2(bd);
		bd2.apply_c(i,clr);
		int64_t total=0;
		for(int i=0;i<montecarlo_playouts;i++){
			Board bd3(bd2);
			total+=playout_o(bd3);
		}
		if(total<mintotal){
			mintotal=total;
			minat=i;
		}
	}
	return minat;
}

int countmoves_o(Board &bd){
	int nposs=0;  // TODO: Fix the poss array size
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]==-1)continue;
		int x=i%SIZE,y=i/SIZE;
		int dx=1,dy=0;
		for(int r=0;r<4;r++){
			if(x+dx>=0&&x+dx<SIZE&&y+dy>=0&&y+dy<SIZE&&bd[i+SIZE*dy+dx]==-1){
				nposs++;
			}

			int t=dx;
			dx=-dy;
			dy=t;
		}
	}
	return nposs;
}

void test(){
	/*for(int distrib=0;distrib<=100;distrib++){
		Board bd;
		int maxval=0;
		int64_t total=0;
		for(int iter=0;iter<100000;iter++){
			for(int i=0;i<SIZE*SIZE;i++){
				bd[i]=rand()%100>=distrib?0:-1;
			}
			int count=countmoves_o(bd);
			maxval=max(maxval,count);
			total+=count;
		}
		// cout<<distrib<<' '<<maxval<<endl;
		cout<<distrib<<' '<<(double)total/100000<<endl;
	}*/

	Board bd;
	int maxval=0;
	for(int64_t i=0;i<((int64_t)1<<(SIZE*SIZE));i++){
		for(int j=0;j<SIZE*SIZE;j++){
			bd[j]=((i>>j)&1)-1;
		}
		int count=countmoves_o(bd);
		if(count>maxval){
			maxval=count;
			cout<<"New max: "<<maxval<<endl;
			bd.print(cout);
		}
	}
}

int main(){
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		srand(tv.tv_sec*1000000+tv.tv_usec);
	}

	const bool debug=isatty(0);

	// test(); return 0;

	if(debug)cerr<<"player? ";
	char c;
	cin>>c;
	if(cin.eof())return 0;
	int player;
	if(tolower(c)=='o'||tolower(c)=='O'){
		player=ORDER;
	} else if(tolower(c)=='c'||tolower(c)=='C'){
		player=CHAOS;
	} else {
		cerr<<"Invalid protocol: first line"<<endl;
		return 1;
	}

	Board bd;
	while(true){
		if(debug)bd.print(cerr);

		cin.ignore(numeric_limits<streamsize>::max(),'\n');
		if(debug)cerr<<"> ";
		if(tolower(cin.peek())=='q')break;

		if(player==CHAOS){
			int clr;
			cin>>clr;
			if(cin.eof())break;
			Move mv={.player=CHAOS,.clr=clr};
			TIMEIT({mv.idx=calcmove_c(bd,clr);});
			if(!bd.validmove(mv)){
				cerr<<"AI calculated invalid move ("<<mv.idx<<" clr="<<mv.clr<<")!"<<endl;
				return 1;
			}
			bd.applymove(mv);
			cout<<mv.idx<<' '<<mv.clr<<endl;
		} else {
			Move mv;
			mv.player=CHAOS;
			cin>>mv.idx>>mv.clr;
			if(cin.eof())break;
			if(!bd.validmove(mv)){
				cerr<<"AI received invalid move ("<<mv.idx<<" clr="<<mv.clr<<")!"<<endl;
				return 1;
			}
			bd.applymove(mv);
		}

		if(debug)bd.print(cerr);

		if(debug)cerr<<"> ";
		if(player==ORDER){
			pair<int,int> p;
			TIMEIT({p=calcmove_o(bd);});
			Move mv={.player=ORDER,.idx=p.first,.idx2=p.second};
			if(!bd.validmove(mv)){
				cerr<<"AI calculated invalid move ("<<mv.idx<<" -> "<<mv.idx2<<")!"<<endl;
				return 1;
			}
			bd.applymove(mv);
			cout<<mv.idx<<' '<<mv.idx2<<endl;
		} else {
			Move mv;
			mv.player=ORDER;
			cin>>mv.idx>>mv.idx2;
			if(cin.eof())break;
			if(!bd.validmove(mv)){
				cerr<<"AI received invalid move ("<<mv.idx<<" -> "<<mv.idx2<<")!"<<endl;
				return 1;
			}
			bd.applymove(mv);
		}
	}
}
