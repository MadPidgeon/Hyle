#include <limits>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <cassert>
#include <unistd.h>
#include <sys/time.h>
#include "lib.h"

using namespace std;


Board::Board(){
	memset(bd,-1,SIZE*SIZE);
}

bool Board::full() const {
	const uint64_t *buf=reinterpret_cast<const uint64_t*>(bd);
	for(int i=0;i<SIZE*SIZE/8;i++){
		if(buf[i]&(uint64_t)0x8080808080808080)return false;
	}
	for(int i=SIZE*SIZE/8*8;i<SIZE*SIZE;i++){
		if(bd[i]<0)return false;
	}
	return true;
}

int Board::nempty() const {
	int num=0;
	for(int i=0;i<SIZE*SIZE;i++)num+=bd[i]==-1;
	return num;
}


__attribute__((unused))
static int scoreO4(const Board &bd){
	int res=0;
	for(int i=0;i<SIZE;i++){
		for(int sz=2;sz<=SIZE;sz++){
			for(int j=0;j<=SIZE-sz;j++){
				int add=sz;
				int base=SIZE*i+j;
				for(int k=0;k<sz/2;k++){
					if(bd[base+k]==-1||bd[base+k]!=bd[base+sz-1-k]){
						add=0;
						break;
					}
				}
				res+=add;

				add=sz;
				base=SIZE*j+i;
				for(int k=0;k<sz/2;k++){
					if(bd[base+k*SIZE]==-1||bd[base+k*SIZE]!=bd[base+(sz-1-k)*SIZE]){
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

__attribute__((unused))
static int scoreline(const int8_t *line){
	int res=0;
	for(int sz=2;sz<=SIZE;sz++){
		for(int i=0;i<=SIZE-sz;i++){
			int add=sz;
			for(int k=0;k<sz/2;k++){
				if(line[i+k]==-1||line[i+k]!=line[i+sz-1-k]){
					add=0;
					break;
				}
			}
			res+=add;
		}
	}
	return res;
}

static int *scorecache;  // array of (SIZE+1)^SIZE scores

__attribute__((constructor))
static void init_scorecache_list(){
	int cachesz=1;
	for(int i=0;i<SIZE;i++){
		cachesz*=SIZE+1;
	}
	scorecache=new int[cachesz];

	int8_t line[SIZE];
	for(int i=0;i<cachesz;i++){
		int num=i;
		for(int j=0;j<SIZE;j++){
			line[j]=num%(SIZE+1)-1;
			num/=SIZE+1;
		}
		scorecache[i]=scoreline(line);
	}
}

int Board::score() const {
	int total=0;
	for(int i=0;i<SIZE;i++){
		int num1=0,num2=0,walk=1;
		for(int j=0;j<SIZE;j++){
			num1+=walk*(bd[SIZE*i+j]+1);
			num2+=walk*(bd[SIZE*j+i]+1);
			walk*=SIZE+1;
		}
		total+=scorecache[num1];
		total+=scorecache[num2];
	}
	return total;
}

bool Board::validmove(const Move &mv) const {
	if(mv.player==ORDER)return valid_o(mv.idx,mv.idx2);
	if(mv.player==CHAOS)return valid_c(mv.idx);
	return false;
}

bool Board::valid_o(int idx,int idx2) const {
	return idx>=0&&idx<SIZE*SIZE&&
	       idx2>=0&&idx2<SIZE*SIZE&&
	       idx!=idx2&&
	       bd[idx]!=-1&&bd[idx2]==-1&&
	       (idx%SIZE==idx2%SIZE||idx/SIZE==idx2/SIZE);
}

bool Board::valid_c(int idx) const {
	return bd[idx]==-1;
}

void Board::applymove(const Move &mv){
	assert(validmove(mv));
	if(mv.player==ORDER)apply_o(mv.idx,mv.idx2);
	else if(mv.player==CHAOS)apply_c(mv.idx,mv.clr);
	else assert(false);
}

void Board::apply_o(int idx,int idx2){
	bd[idx2]=bd[idx];
	bd[idx]=-1;
}

void Board::apply_c(int idx,int8_t clr){
	bd[idx]=clr;
}

void Board::undo_o(int idx,int idx2){
	bd[idx]=bd[idx2];
	bd[idx2]=-1;
}

void Board::undo_c(int idx){
	bd[idx]=-1;
}

void Board::print(ostream &os) const {
	const char *dict[]={
		".", "\x1B[41m0\x1B[0m", "\x1B[42m1\x1B[0m", "\x1B[44m2\x1B[0m", "\x1B[43m3\x1B[0m",
		"\x1B[46m4\x1B[0m", "\x1B[45m5\x1B[0m", "\x1B[47m6\x1B[0m",
	};
	for(int y=0;y<SIZE;y++){
		for(int x=0;x<SIZE;x++){
			if(x!=0)os<<' ';
			int8_t v=bd[SIZE*y+x];
			if(v>=-1&&v<=6)os<<dict[v+1];
			else os<<v;
		}
		os<<'\n';
	}
	os.flush();
}

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

int randmove_c(Board &bd){
	int poss[SIZE*SIZE],nposs=0;
	for(int i=0;i<SIZE*SIZE;i++){
		if(bd[i]==-1)poss[nposs++]=i;
	}
	if(nposs==0)return -1;
	return poss[rand()%nposs];
}

int main(){
	{
		struct timeval tv;
		gettimeofday(&tv,NULL);
		srand(tv.tv_sec*1000000+tv.tv_usec);
	}

	const bool debug=isatty(0);

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
