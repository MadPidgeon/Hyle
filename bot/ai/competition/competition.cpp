#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <string>
#include <tuple>
#include <stdexcept>
#include <functional>
#include <cstring>
#include <cstdint>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/wait.h>

using namespace std;


class Defer{
	function<void(void)> cb;
public:
	Defer(const function<void(void)> &cb):cb(cb){}
	~Defer(){
		cb();
	}
};

static void die(const string &msg){
	cerr<<msg<<endl;
	throw runtime_error(msg);
}

static void dieperror(const string &func){
	const char *msg=strerror(errno);
	die(func+": "+msg);
}

__attribute__((unused))
static string tempname(const string &base){
	return ".competition."+to_string(getpid())+".tmp."+base;
}

static void mkdirp(const char *name){
	struct stat statbuf;
	if(stat(name,&statbuf)<0){
		if(errno!=ENOENT){
			dieperror("stat");
		}
	}
	if((statbuf.st_mode&S_IFMT)!=S_IFDIR){
		if(mkdir(name,0777)<0){
			dieperror("mkdir");
		}
	}
}

__attribute__((unused))
static void mkfifop(const string &name){
	if(mkfifo(name.c_str(),0777)<0){
		dieperror("mkfifo");
	}
}

static string sbasename(const string &path){
	const char *data=path.c_str();
	const char *slash=strrchr(data,'/');
	const char *start=slash==NULL?data:slash+1;
	return start;
}

class Spawn{
public:
	enum class Type {
		pipe,
		inherit,
		redirect,
	};
	const Type type;
	const string fname;

private:
	Spawn(Type type):type(type){}
	Spawn(Type type,const string &fname):type(type),fname(fname){}

public:
	static Spawn pipe;
	static Spawn inherit;
	static Spawn redirect(const string &fname);
};

Spawn Spawn::pipe=Spawn(Type::pipe);
Spawn Spawn::inherit=Spawn(Type::inherit);
Spawn Spawn::redirect(const string &fname){
	return Spawn(Type::redirect,fname);
}

pair<pid_t,array<int,3>> spawn(const vector<string> &args,array<Spawn,3> ss){
	if(args.size()==0){
		die("Invaild args size in spawn()");
	}
	char **cargs=new char*[args.size()+1];
	for(size_t i=0;i<args.size();i++){
		cargs[i]=new char[args[i].size()+1];
		memcpy(cargs[i],args[i].c_str(),args[i].size()+1);
	}
	cargs[args.size()]=NULL;

	int pipes[3][2]={{-1}};
	int filefds[3]={-1};
	for(int i=0;i<3;i++){
		if(ss[i].type==Spawn::Type::pipe){
			if(pipe(pipes[i])<0)dieperror("pipe");
		} else if(ss[i].type==Spawn::Type::redirect){
			filefds[i]=open(ss[i].fname.c_str(),i==0?O_RDONLY:O_WRONLY|O_TRUNC);
			if(filefds[i]<0)dieperror("open");
		}
	}

	pid_t pid=fork();
	if(pid<0){
		dieperror("fork");
	}
	if(pid==0){
		for(int i=0;i<3;i++){
			if(ss[i].type==Spawn::Type::pipe){
				close(pipes[i][i==0?1:0]);
				dup2(pipes[i][i==0?0:1],i);
				close(pipes[i][i==0?0:1]);
			} else if(ss[i].type==Spawn::Type::redirect){
				dup2(filefds[i],i);
				close(filefds[i]);
			}
		}
		execv(cargs[0],cargs);
		exit(234);  // ?
	}

	array<int,3> triple({-1,-1,-1});

	for(int i=0;i<3;i++){
		if(ss[i].type==Spawn::Type::pipe){
			close(pipes[i][i==0?0:1]);
			triple[i]=pipes[i][i==0?1:0];
		} else if(ss[i].type==Spawn::Type::redirect){
			close(filefds[i]);
		}
	}

	return {pid,triple};
}

static int64_t gettimestamp(){
	struct timeval tv;
	gettimeofday(&tv,NULL);
	return (int64_t)tv.tv_sec*1000000+tv.tv_usec;
}

static function<void(void)> makeprocdefer(pid_t pid){
	return [pid](){
		kill(pid,SIGCONT);
		kill(pid,SIGTERM);
		usleep(100000);
		int status;
		if(waitpid(pid,&status,0)==pid&&!WIFEXITED(status)){
			cerr<<"Had to SIGKILL process "<<pid<<endl;
			kill(pid,SIGKILL);
		}
	};
}

static void playgame(const string &p1,const string &p2,const int gameidx=1){
	const string p1b=sbasename(p1),p2b=sbasename(p2);

	ofstream complog("competitionlogs/"+p1b+"--"+p2b+"."+to_string(gameidx)+".txt");
	if(!complog){
		die("Cannot open competition log!");
	}

	complog<<"MANAGER for "<<p1b<<" vs "<<p2b<<" (game "<<gameidx<<")"<<endl;
	cout<<p1b<<" vs "<<p2b<<" (game "<<gameidx<<"): "<<flush;

	const string p1errname="playerlogs/"+p1b+"."+p1b+"--"+p2b+"."+to_string(gameidx)+".txt";
	const string p2errname="playerlogs/"+p2b+"."+p1b+"--"+p2b+"."+to_string(gameidx)+".txt";

	pid_t refpid;
	array<int,3> reffds;
	tie(refpid,reffds)=
		spawn({"../referee",p1,p2},{Spawn::pipe,Spawn::pipe,Spawn::inherit});
	Defer ref_defer(makeprocdefer(refpid));
	FILE *refin=fdopen(reffds[0],"w"),*refout=fdopen(reffds[1],"r");
	if(!refin||!refout)dieperror("fdopen");

	struct PlayerData {
		pid_t pid;
		array<int,3> fds;
		FILE *in,*out;
		double timing;
	};

	PlayerData players[2];

	tie(players[0].pid,players[0].fds)=
		spawn({p1},{Spawn::pipe,Spawn::pipe,Spawn::redirect(p1errname)});
	Defer p1_defer(makeprocdefer(players[0].pid));
	players[0].in=fdopen(players[0].fds[0],"w");
	players[0].out=fdopen(players[0].fds[1],"r");
	if(!players[0].in||!players[0].out)dieperror("fdopen");
	players[0].timing=0;

	kill(players[0].pid,SIGSTOP);

	tie(players[1].pid,players[1].fds)=
		spawn({p2},{Spawn::pipe,Spawn::pipe,Spawn::redirect(p2errname)});
	Defer p2_defer(makeprocdefer(players[1].pid));
	players[1].in=fdopen(players[1].fds[0],"w");
	players[1].out=fdopen(players[1].fds[1],"r");
	if(!players[1].in||!players[1].out)dieperror("fdopen");
	players[1].timing=0;

	kill(players[1].pid,SIGSTOP);

	const char *reason=NULL;

	char *line=NULL;
	size_t linesz=0;
	while(true){
		int status;
		if(waitpid(refpid,&status,WNOHANG)>0&&WIFEXITED(status)){
			reason="Referee quit";
			break;
		}
		if(waitpid(players[0].pid,&status,WNOHANG)>0&&WIFEXITED(status)){
			reason="Player 1 quit";
			break;
		}
		if(waitpid(players[1].pid,&status,WNOHANG)>0&&WIFEXITED(status)){
			reason="Player 2 quit";
			break;
		}

		ssize_t nread=::getline(&line,&linesz,refout);
		if(nread<0)die("Failure to read a line from the referee!");
		line[--nread]='\0';

		char *spptr=(char*)memchr(line,' ',nread);
		size_t spidx;
		if(spptr==NULL){
			spidx=nread;
		} else {
			spidx=spptr-line;
			line[spidx]='\0';
		}

		char *cmd=line;
		char *args=spidx==(size_t)nread?line+spidx:line+spidx+1;
		size_t argslen=line+nread-args;

		// cerr<<"cmd=<<"<<cmd<<">> args=<<"<<args<<">>"<<endl;

		if(strcmp(cmd,"writeln")==0){
			if(argslen<2)die("Invalid 'writeln' line from referee");
			args[argslen]='\n';
			if(args[0]!='1'&&args[0]!='2'){
				die("Invalid 'writeln' target from referee");
			}
			FILE *target=players[args[0]-'1'].in;
			fwrite(args+2,1,argslen-1,target);
			fflush(target);
			args[argslen]='\0';
		} else if(strcmp(cmd,"readln")==0){
			if(strcmp(args,"1")!=0&&strcmp(args,"2")!=0){
				die("Invalid 'readln' target from referee");
			}
			int target=args[0]-'1';

			char *rline=NULL;
			size_t rlinesz=0;
			kill(players[target].pid,SIGCONT);
			int64_t tstart=gettimestamp();
			ssize_t rnread=getline(&rline,&rlinesz,players[target].out);
			int64_t tend=gettimestamp();
			kill(players[target].pid,SIGSTOP);
			players[target].timing+=(double)(tend-tstart)/1000000;
			if(rnread<0){
				if(feof(players[target].out)){
					die(string("EOF on stdout of p")+args);
				} else {
					die(string("Error reading from p")+args+": "+strerror(errno));
				}
			}
			fwrite(rline,1,rnread,refin);
			fflush(refin);
			free(rline);
		} else if(strcmp(cmd,"writelog")==0){
			complog.write(args,argslen);
			complog.put('\n');
		} else if(strcmp(cmd,"timings")==0){
			fprintf(refin,"%lf %lf\n",players[0].timing,players[1].timing);
			fflush(refin);
		} else if(strcmp(cmd,"results")==0){
			complog<<"\nFINAL SCORES: ";
			complog.write(args,argslen);
			complog.put('\n');
			cout.write(args,argslen);
			cout.put('\n');
			break;
		} else if(strcmp(cmd,"invalid")==0){
			complog<<"\nINVALID: ";
			complog.write(args,argslen);
			complog.put('\n');
			cout<<"invalid"<<endl;
		}
	}

	if(line!=NULL)free(line);

	if(reason!=NULL){
		die(reason);
	}
}

int main(int argc,char **argv){
	vector<string> players;
	for(int i=1;i<argc;i++)players.push_back(argv[i]);

	const int nplayers=players.size();
	if(nplayers==0){
		cerr<<"Usage: "<<argv[0]<<" <players...>"<<endl;
		return 1;
	}

	mkdirp("playerlogs");
	mkdirp("competitionlogs");

	for(int i1=0;i1<nplayers;i1++){
		for(int i2=0;i2<nplayers;i2++){
			if(i1==i2)continue;
			playgame(players[i1],players[i2]);
		}
	}
}
