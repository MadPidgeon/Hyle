#!/usr/bin/env node
const child_process=require("child_process");
const socketio=require("socket.io-client");

if(process.argv.length!=4){
	console.log("Usage: proxy.js <./ai_executable> <http://server.url>");
	process.exit(1);
}
const arg_ai_path=process.argv[2];
const arg_server_url=process.argv[3];

const SIZE=7;
const ORDER=0,CHAOS=1;

const socket=socketio(arg_server_url);
let aiproc=null;

let player=null;
let board=null;

let lastboarddate=new Date();

function aiwrite(line){
	console.log(`AI write: ${line}`);
	aiproc.stdin.write(line+"\n");
}

socket.on("connect",()=>{
	console.log("Connected, creating game");
	socket.emit("size",(size)=>{
		if(size!=SIZE){
			console.log("Server configured with unsupported size!");
			process.exit(1);
		}
		board=new Array(SIZE).fill(0).map(_=>new Array(SIZE).fill(-1));
		socket.emit("create",(id)=>{
			console.log(`<< id = ${id} >>`);
		});
	});
});

socket.on("disconnect",()=>{
	console.log("Disconnected");
	if(aiproc)aiproc.kill();
	process.exit(0);
});

socket.on("gamestart",(p_)=>{
	player=p_;
	console.log(`Game start (player=${player})`);
	aiproc=child_process.spawn(arg_ai_path,{
		stdio: ["pipe","pipe",process.stderr],
	});
	aiproc.on("exit",(code,sig)=>{
		console.log(`AI process terminated: ${code}/${sig}`);
		process.exit(1);
	});
	aiwrite(player==0?"o":"c");

	let buffer="";
	aiproc.stdout.on("data",(data)=>{
		buffer+=data;
		let idx=buffer.indexOf("\n");
		while(idx!=-1){
			const line=buffer.slice(0,idx);
			ai_line(line);
			buffer=buffer.slice(idx+1);
			idx=buffer.indexOf("\n");
		}
	});
});

socket.on("board",(nb)=>{
	lastboarddate=new Date();
	nb=JSON.parse(nb);
	let diff=[];
	let full=true;
	for(let y=0;y<SIZE;y++){
		for(let x=0;x<SIZE;x++){
			if(nb[y][x]!=board[y][x]){
				diff.push([x,y]);
			}
			if(nb[y][x]==-1)full=false;
		}
	}
	console.log(`diff = ${diff}`);
	if(full==true){
		console.log("Board full!");
		return;
	}
	if(diff.length==0)return;
	if(diff.length==1){
		let x=diff[0][0],y=diff[0][1];
		aiwrite((y*SIZE+x)+" "+nb[y][x]);
		board[y][x]=nb[y][x];
	}
	if(diff.length==2){
		let x1=diff[0][0],y1=diff[0][1],x2=diff[1][0],y2=diff[1][1];
		if(board[y1][x1]!=-1){
			aiwrite((y1*SIZE+x1)+" "+(y2*SIZE+x2));
			board[y2][x2]=board[y1][x1];
			board[y1][x1]=-1;
		} else {
			aiwrite((y2*SIZE+x2)+" "+(y1*SIZE+x1));
			board[y1][x1]=board[y2][x2];
			board[y2][x2]=-1;
		}
	}
	if(diff.length>2){
		console.log("More than 2 differences in board; proxy/server failure?");
		if(aiproc)aiproc.kill();
		process.exit(1);
	}
});

socket.on("turn",(clr)=>{
	if(clr!=null){
		aiwrite(clr);
	}
});

socket.on("err",(s)=>{
	console.log(`Err: ${s}`);
	if(aiproc)aiproc.kill();
	process.exit(1);
});


function ai_line(line){
	console.log("AI read: "+line);
	line=line.split(" ").map(v=>parseInt(v,10));
	let msg;
	if(player==ORDER){
		msg="omove";
		board[~~(line[1]/SIZE)][line[1]%SIZE]=board[~~(line[0]/SIZE)][line[0]%SIZE];
		board[~~(line[0]/SIZE)][line[0]%SIZE]=-1;
	} else {
		msg="cmove";
		board[~~(line[0]/SIZE)][line[0]%SIZE]=line[1];
	}
	setTimeout(
		()=>socket.emit(msg,line[0],line[1]),
		Math.max(0,1000-(new Date()-lastboarddate)));
}
