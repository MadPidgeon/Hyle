#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include <sys/time.h>
#include "lib.h"

using namespace std;


static const int minimax_depth=2;

static int evaluate(const Board &bd){
	// TODO: explore "guaranteed points"
	return bd.score();
}

static int minimax_o(Board &bd,int depth,int alpha,int beta);

static int minimax_c_notfull(Board &bd,int8_t clr,int depth,int alpha,int beta){
	for(int idx=0;idx<SIZE*SIZE;idx++){
		if(bd[idx]!=-1)continue;
		bd.apply_c(idx,clr);
		int sc=minimax_o(bd,depth-1,alpha,beta);
		bd.undo_c(idx,clr);

		if(sc<beta){
			beta=sc;
			if(beta<=alpha)return beta;
		}
	}

	return beta;
}

static int minimax_clr(Board &bd,int depth,int alpha,int beta){
	bool isfull=bd.full();
	if(depth<=0||isfull)return evaluate(bd);

	int sc=0,total=0;
	for(int8_t clr=0;clr<SIZE;clr++){
		int weight=bd.bagleft(clr);
		if(weight==0)continue;
		sc+=weight*minimax_c_notfull(bd,clr,depth,alpha,beta);
		total+=weight;
	}
	return sc/total;
}

static int minimax_o(Board &bd,int depth,int alpha,int beta){
	bool isfull=bd.full();
	if(depth<=0||isfull)return evaluate(bd);

	int idx1,idx2;
	FOR_ALL_ORDER_MOVES(bd,idx1,idx2) {
		bd.apply_o(idx1,idx2);
		int sc=minimax_clr(bd,depth-1,alpha,beta);
		bd.undo_o(idx1,idx2);

		if(sc>alpha){
			alpha=sc;
			if(alpha>=beta)return alpha;
		}
	}

	return alpha;
}

pair<int,int> calcmove_o(const Board &bd_){
	Board bd(bd_);

	int maxsc=INT_MIN+1;
	pair<int,int> maxat(-1,-1);
	int idx1,idx2;
#ifdef DO_RAISE_DEPTH
	int freespaces=bd.nempty();
#endif
	FOR_ALL_ORDER_MOVES(bd,idx1,idx2) {
		bd.apply_o(idx1,idx2);
		int sc,dep=minimax_depth;
#ifdef DO_RAISE_DEPTH
		do {
			clock_t start=clock();
#endif
			sc=minimax_clr(bd,dep,INT_MIN+1,INT_MAX);
#ifdef DO_RAISE_DEPTH
			clock_t end=clock();
			if(end-start>CLOCKS_PER_SEC/50)break;
			dep++;
		} while(dep<=2*freespaces);
#endif
		bd.undo_o(idx1,idx2);
		cerr<<sc<<"("<<idx1<<">"<<idx2;
		for(int i=minimax_depth;i<dep;i++)cerr<<"^";
		cerr<<") ";
		if(sc>maxsc){
			maxsc=sc;
			maxat={idx1,idx2};
		}
	}
	cerr<<endl;

	return maxat;
}

int calcmove_c(const Board &bd_,int clr){
	Board bd(bd_);

	int minsc=INT_MAX;
	int minat=-1;
#ifdef DO_RAISE_DEPTH
	int freespaces=bd.nempty();
#endif
	for(int idx=0;idx<SIZE*SIZE;idx++){
		if(bd[idx]!=-1){
			cerr<<".,";
			continue;
		}
		bd.apply_c(idx,clr);
		int sc,dep=minimax_depth;
#ifdef DO_RAISE_DEPTH
		do {
			clock_t start=clock();
#endif
			sc=minimax_o(bd,dep,INT_MIN+1,INT_MAX);
#ifdef DO_RAISE_DEPTH
			clock_t end=clock();
			if(end-start>CLOCKS_PER_SEC/50)break;
			dep++;
		} while(dep<=2*(freespaces-1)+1);
#endif
		bd.undo_c(idx,clr);
		cerr<<sc;
		for(int i=minimax_depth;i<dep;i++)cerr<<"^";
		cerr<<",";
		if(sc<minsc){
			minsc=sc;
			minat=idx;
		}
	}
	cerr<<endl;

	return minat;
}

int main(int argc,char **argv){
	return lib_main(argc,argv);
}
