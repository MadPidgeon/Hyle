#pragma once

#include <iostream>
#include <utility>
#include <ctime>

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
	int idx,idx2;
	int clr;
};

class Board{
public:
	int8_t bd[SIZE*SIZE];
	int8_t bag[SIZE];
	
	Board();

	inline int8_t& operator[](int idx){return bd[idx];}

	inline int8_t operator[](int idx) const {return bd[idx];}

	bool full() const;
	int nempty() const;
	int score() const;
	bool validmove(const Move &mv) const;
	bool valid_o(int idx,int idx2) const;
	inline bool valid_c(int idx) const {return idx>=0&&idx<SIZE*SIZE&&bd[idx]==-1;}

	void applymove(const Move &mv);
	inline void apply_o(int idx,int idx2){bd[idx2]=bd[idx]; bd[idx]=-1;}
	inline void apply_c(int idx,int8_t clr){bd[idx]=clr; bag[clr]--;}
	inline void undo_o(int idx,int idx2){bd[idx]=bd[idx2]; bd[idx2]=-1;}
	inline void undo_c(int idx,int8_t clr){bd[idx]=-1; bag[clr]++;}

	inline int bagleft(int8_t clr) const {return bag[clr];}

	void print(ostream &os) const;
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


pair<int,int> randmove_o(const Board &bd);
int randmove_c(const Board &bd);

int lib_main(int argc,char **argv);

// TO IMPLEMENT:
pair<int,int> calcmove_o(const Board &bd);
int calcmove_c(const Board &bd,int clr);
