#include <iostream>
#include <vector>
#include <utility>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include "lib.h"

using namespace std;


pair<int,int> calcmove_o(const Board &bd){
	vector<pair<int,int>> poss;
	poss.reserve(max_order_moves(SIZE));
	int idx1,idx2;
	FOR_ALL_ORDER_MOVES(bd,idx1,idx2) {
		poss.emplace_back(idx1,idx2);
	}
	if(poss.size()==0)return {-1,-1};
	return poss[rand()%poss.size()];
}

int calcmove_c(const Board &bd,int clr){
	(void)clr;
	vector<int> poss;
	poss.reserve(SIZE*SIZE);
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]!=-1)continue;
		poss.push_back(i);
	}
	if(poss.size()==0)return -1;
	return poss[rand()%poss.size()];
}

int main(int argc,char **argv){
	return lib_main(argc,argv);
}
