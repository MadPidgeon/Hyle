#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include "lib.h"

using namespace std;


static int playout_o(Board &bd,int left){
	while(left>0){
		pair<int,int> p=randmove_o(bd);
		bd.apply_o(p.first,p.second);

		int idx=randmove_c(bd);
		bd.apply_c(idx,rand()%SIZE);
		left--;
	}
	return bd.score();
}

static int playout_o(Board &bd){
	return playout_o(bd,bd.nempty());
}

static int playout_c(Board &bd){
	int poss[SIZE*SIZE];
	int left=0;
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]==-1)poss[left++]=i;
	}
	if(left==0)return -1;
	bd.apply_c(poss[rand()%left],rand()%SIZE);
	return playout_o(bd,left-1);
}

static const int montecarlo_playouts=10000;

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
			total+=playout_c(bd3);
		}
		if(total>maxtotal){
			maxtotal=total;
			maxat[0]=idx1;
			maxat[1]=idx2;
		}
	}
	return {maxat[0],maxat[1]};
}

int calcmove_c(const Board &bd,int clr){
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

__attribute__((unused))
static int countmoves_o(Board &bd){
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

__attribute__((unused))
static void test(){
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
