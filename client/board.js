const board_size = 7;
const socket = io();
let user_id;
let square_val;
let square_div;
let select = [-1,-1];
let player = 0;
let turn = 0;
let has_turn = false;
let chaos_col;

function getScore() {
	let dx = 0;
	let dy = 1;
	let score = 0;
	for( let dir = 0; dir < 2; ++dir ) {
		for( let x = 0; x < board_size; ++x ) {
			for( let y = 0; y < board_size; ++y ) {
				let x2 = x+dx;
				let y2 = y+dy;
				while( x2 < board_size && y2 < board_size ) {
					let success = true;
					for( let inner = 0; inner <= Math.max(x2-x,y2-y)/2; ++inner ) {
						if( ( square_val[x+inner*dx][y+inner*dy] != square_val[x2-inner*dx][y2-inner*dy] ) || square_val[x+dx*inner][y+dy*inner] == -1 ) {
							success = false;
							break;
						}
					}
					if( success )
						score += x2 + y2 - x - y + 1;
					x2 += dx;
					y2 += dy;
				}
			}
		}
		dx = 1;
		dy = 0;
	}
	return score;
}

function createGame() {
	console.log("Game create!");
	socket.emit("create",function success( id ) {
		user_id = id;
		document.getElementById("connection_info").innerHTML = "Your session id is " + id;
	});
}

function joinGame() {
	console.log("Game join!");
	user_id = document.getElementById("player_hash").value;
	socket.emit("join", user_id );
}

function startGame( p ) {
	document.getElementById("connection_info").innerHTML = "";
	console.log("Game start!");
	square_val = Array( board_size ).fill(0).map(_=>Array( board_size ).fill(-1));
	document.getElementById("game_space").classList.remove("invisible");
	document.getElementById("settings").classList.add("invisible");
	player = p;
	document.getElementById("player_display").innerHTML = p ? "Player: Chaos" : "Player: Order";
	if( p == 1 )
		document.getElementById("chaos_info").className = "";
}
 
function receiveBoard( str ) {
	let receive = JSON.parse( str );
	console.log("Board update: " );
	console.log(square_val);
	console.log(receive);
	for( let x = 0; x < board_size; ++x ) {
		for( let y = 0; y < board_size; ++y ) {
			square_div[x][y].className = "square";
			let inner = square_div[x][y].firstElementChild;
			if( receive[y][x] != -1 ) {
				inner.className = "inner_circle t"+receive[y][x];
			} else {
				inner.className = "invisible";
			}
			if( receive[y][x] != square_val[x][y] )
				square_div[x][y].classList.add("last_play");
		}
	}
	square_val = new Array(board_size).fill(0).map((_,x)=>receive.map(r=>r[x]));
	document.getElementById("order_score").innerHTML = "Score: "+getScore();
	document.getElementById("turn_display").innerHTML = "Turn: " + turn;
}

function startTurn( col ) {
	console.log("Your turn!");
	turn++;
	if( player == 1 ) {
		document.getElementById("chaos_info").classList.remove("invisible");
		chaos_col = col;
		document.getElementById("player_color").className = "inner_circle t" + col;
	}
	has_turn = true;
	document.getElementById("status_message").innerHTML = "Your turn.";
}

function clickSquare( x, y ) {
	console.log("clickSquare(" + x + "," + y + ")");
	if( has_turn ) {
		if( player == 1 ) {
			chaosPlay( [x, y] );
		} else {
			if( select[0] != -1 ) {
				square_div[select[0]][select[1]].classList.remove("select");
				orderPlay( select, [x,y] );
				select = [-1,-1];
			} else {
				if( square_val[x][y] != -1 ) {
					square_div[x][y].classList.add("select");
					select = [x,y];
				}
			}
		}
	}
}

function chaosPlay( loc ) {
	console.log("clickSquare([" + loc[0] + "," + loc[1] + "])");
	if( square_val[loc[0]][loc[1]] != -1 )
		return;
	socket.emit("cmove",loc[1]*board_size+loc[0],chaos_col);
	document.getElementById("status_message").innerHTML = "Opponent is moving...";
	has_turn = false;
	document.getElementById("chaos_info").classList.add("invisible");
}

function orderPlay( loc1, loc2 ) {
	console.log("orderPlay(["+loc1[0]+","+loc1[1]+"], ["+loc2[0]+","+loc2[1]+"])");
	if( (loc1[0] == loc2[0]) == (loc1[1] == loc2[1]) )
		return;
	if( square_val[loc1[0]][loc1[1]] == -1 )
		return;
	console.log("First check!");
	let diff = [Math.sign(loc2[0]-loc1[0]),Math.sign(loc2[1]-loc1[1])];
	let start = [loc1[0],loc1[1]];
	// start = start.map((v,i)=>v+diff[i]);
	let itr = 0;
	do {
		start = start.map((v,i)=>v+diff[i]);
		if( square_val[start[0]][start[1]] != -1 ) {
			console.log(start);
			return;
		}
		if( itr++ > 100 )
			return;
	} while( start[0] != loc2[0] || start[1] != loc2[1] );
	socket.emit("omove",loc1[1]*board_size+loc1[0],loc2[1]*board_size+loc2[0]);
	document.getElementById("status_message").innerHTML = "Opponent is moving...";
	has_turn = false;
}

function loadBoard() {
	let container = document.getElementById("board_container");
	square_div = Array( board_size ).fill(0).map(_=>Array( board_size, 0 ) );
	for( let y = 0; y < board_size; ++y ) {
		let row_name = "r" + y;
		let row = document.createElement("div");
		row.id = row_name;
		row.className = "row";
		container.appendChild(row);
		row = document.getElementById(row_name);
		for( let x = 0; x < board_size; ++x ) {
			let square_name = "s" + y + x;
			let square = document.createElement("div");
			square.innerHTML = "<div class='inner_circle invisible'></div>";
			square.id = square_name;
			square.className = "square";
			square.onclick = () => clickSquare(x,y); 
			row.appendChild(square);
			square_div[x][y] = document.getElementById(square_name);
		}
	}
}



window.onload = function() {
	socket.on("gamestart",startGame);
	socket.on("board",receiveBoard);
	socket.on("disconnect", ()=>{location.href = location.href;});
	socket.on("turn",startTurn);
	socket.on("err",(str)=>alert(str));
	loadBoard();
}
