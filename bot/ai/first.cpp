#include <iostream>
#include <cstdlib>
#include <cstdint>
#include <climits>
#include "lib.h"

using namespace std;


pair<int,int> calcmove_o(const Board &bd){
	int idx1,idx2;
	FOR_ALL_ORDER_MOVES(bd,idx1,idx2) {
		return {idx1,idx2};
	}
	return {-1,-1};
}

int calcmove_c(const Board &bd,int clr){
	(void)clr;
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]!=-1)continue;
		return i;
	}
	return -1;
}

int main(int argc,char **argv){
	return lib_main(argc,argv);
}
