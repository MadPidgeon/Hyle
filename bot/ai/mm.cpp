#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include "lib.h"

using namespace std;


static const int minimax_depth=4;

static int evaluate(const Board &bd){
	return bd.score();
}

static int minimax_o(Board &bd,int depth,int alpha,int beta);

static int minimax_c_notfull(Board &bd,int8_t clr,int depth,int alpha,int beta){
	for(int idx=0;idx<SIZE*SIZE;idx++){
		if(bd[idx]!=-1)continue;
		bd.apply_c(idx,clr);
		int sc=minimax_o(bd,depth-1,alpha,beta);
		bd.undo_c(idx);

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

	int total=0;
	for(int8_t clr=0;clr<SIZE;clr++){
		total+=minimax_c_notfull(bd,clr,depth,alpha,beta);
	}
	return total/SIZE;
}

static int minimax_o(Board &bd,int depth,int alpha,int beta){
	// cerr<<"minimax_o(depth="<<depth<<",alpha="<<alpha<<",beta="<<beta<<")"<<endl;
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
	FOR_ALL_ORDER_MOVES(bd,idx1,idx2) {
		bd.apply_o(idx1,idx2);
		int sc=minimax_clr(bd,minimax_depth,INT_MIN+1,INT_MAX);
		bd.undo_o(idx1,idx2);
		cerr<<idx1<<">"<<idx2<<"="<<sc<<" ";
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
	for(int idx=0;idx<SIZE*SIZE;idx++){
		if(bd[idx]!=-1){
			cerr<<".,";
			continue;
		}
		bd.apply_c(idx,clr);
		int sc=minimax_o(bd,minimax_depth,INT_MIN+1,INT_MAX);
		bd.undo_c(idx);
		cerr<<sc<<",";
		if(sc<minsc){
			minsc=sc;
			minat=idx;
		}
	}
	cerr<<endl;

	return minat;
}
