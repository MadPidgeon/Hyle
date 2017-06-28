#!/usr/bin/env node
const express=require("express");
const app=express();
const http=require("http").Server(app);
const io=require("socket.io")(http);
const path=require("path");

const PORT=8080;
const BOARD_SIZE=7;

const clientdir=path.normalize(__dirname+"/../client");

function validNum(v,min,max){
	if(typeof v!="number"||isNaN(v)||v%1!=0||v<min||v>=max){
		throw new Error("Invalid argument");
	}
}

function shuffleArray(array) {
	for (var i = array.length - 1; i > 0; i--) {
		var j = Math.floor(Math.random() * (i + 1));
		var temp = array[i];
		array[i] = array[j];
		array[j] = temp;
	}
}

const ORDER=0,CHAOS=1;

function randomClr(sz){
	return Math.random()*sz|0;
}

class Board{
	constructor(sz){
		validNum(sz,1,1000);
		this.sz=sz;
		this.bd=new Array(sz*sz).fill(-1);
		this.bd[17]=3;
		this.turn=CHAOS;
	}

	size(){return this.sz;}

	free(idx){
		validNum(idx,0,this.sz*this.sz);
		return this.bd[idx]==-1;
	}

	add(idx,clr){
		if(this.turn!=CHAOS){
			throw new Error("Chaos is not on turn");
		}
		validNum(idx,0,this.sz*this.sz);
		validNum(clr,0,this.sz);
		if(this.bd[idx]!=-1){
			throw new Error("Index already taken");
		}
		this.bd[idx]=clr;
		this.turn=ORDER;
	}

	move(from,to){
		if(this.turn!=ORDER){
			throw new Error("Order is not on turn");
		}
		validNum(from,0,this.sz*this.sz);
		validNum(to,0,this.sz*this.sz);
		if(from==to){
			throw new Error("From == to");
		}
		if(bd[from]==-1||bd[to]!=-1){
			throw new Error("Invalid move");
		}
		bd[to]=bd[from];
		bd[from]=-1;
		this.turn=CHAOS;
	}

	serialise(){
		const res=[];
		for(let y=0;y<this.sz;y++){
			res.push(this.bd.slice(this.sz*y,this.sz*(y+1)));
		}
		return JSON.stringify(res);
	}
};

{
	let id=1;
	function uniqid(){
		return id++;
	}
}

function closeGame(id){
	const game=games.get(id);
	for(const c of game.conns)c.conn.disconnect("close");
	games.delete(id);
}

const games=new Map();

io.on("connection",(conn)=>{
	let cobj={
		game: null,
		conn: conn,
	};
	console.log("Connect");

	conn.on("disconnect",()=>{
		console.log("Disconnect: "+(cobj.game?cobj.game.id:"(n.a.)"));
		if(cobj.game!=null)closeGame(cobj.game.id);
	});
	conn.on("size",(cb)=>{
		cb(BOARD_SIZE);
	});
	conn.on("create",(cb)=>{
		console.log("Create:");
		if(cobj.game!=null){
			console.log("  error");
			conn.emit("err","Already in game");
			return;
		}
		cobj.game={
			id: uniqid(),
			conns: [cobj],
			bd: new Board(BOARD_SIZE),
			chaosclr: randomClr(BOARD_SIZE),
		};
		console.log("  id="+cobj.game.id);
		games.set(cobj.game.id,cobj.game);
		cb(cobj.game.id);
	});
	conn.on("join",(in_id)=>{
		in_id=+in_id;
		console.log("Join: "+in_id);
		if(cobj.game!=null){
			conn.emit("err","Already in game");
			return;
		}
		cobj.game=games.get(in_id);
		if(cobj.game==null){
			conn.emit("err","Game not found");
			return;
		}
		cobj.game.conns.push(cobj);
		if(cobj.game.conns.length!=2){
			throw new AssertionError("???");
		}
		shuffleArray(cobj.game.conns);
		for(let i=0;i<cobj.game.conns.length;i++){
			cobj.game.conns[i].conn.emit("gamestart",i);
			cobj.game.conns[i].conn.emit("board",cobj.game.bd.serialise());
		}
		cobj.game.conns[CHAOS].conn.emit("turn",cobj.game.chaosclr);
	});
	conn.on("cmove",(idx,clr)=>{
		console.log("Cmove: "+(cobj.game?cobj.game.id:"(n.a.)")+" ("+idx+","+clr+")");
		if(cobj.game==null){
			conn.emit("err","Not in a game");
			return;
		}
		if(clr!=cobj.game.chaosclr){
			conn.emit("err","Not the assigned chaos colour");
			return;
		}
		try {
			cobj.game.bd.add(idx,clr);
		} catch(e){
			conn.emit("err",e.message);
			return;
		}
		cobj.game.chaosclr=randomClr(BOARD_SIZE);
		const ser=cobj.game.bd.serialise();
		for(const o of cobj.game.conns){
			o.conn.emit("board",ser);
		}
		cobj.game.conns[ORDER].conn.emit("turn");
	});
	conn.on("omove",(idx1,idx2)=>{
		console.log("Cmove: "+(cobj.game?cobj.game.id:"(n.a.)")+" ("+idx1+","+idx2+")");
		if(cobj.game==null){
			conn.emit("err","Not in a game");
			return;
		}
		try {
			cobj.game.bd.move(idx1,idx2);
		} catch(e){
			conn.emit("err",e.message);
			return;
		}
		const ser=cobj.game.bd.serialise();
		for(const o of cobj.game.conns){
			o.conn.emit("board",ser);
		}
		cobj.game.conns[CHAOS].conn.emit("turn",cobj.game.chaosclr);
	});
});

app.get("/",(req,res)=>{
	res.sendFile(clientdir+"/index.html");
});

app.use(express.static(clientdir));

http.listen(PORT,()=>{
	console.log("Listening on port "+PORT);
});
