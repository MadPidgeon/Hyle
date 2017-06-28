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

function createGame() {
	console.log("Game create!");
	socket.emit("create",function success( id ) {
		user_id = id;
	});
}

function joinGame() {
	console.log("Game join!");
	user_id = document.getElementById("player_hash").value;
	socket.emit("join", user_id );
}

function startGame( p ) {
	console.log("Game start!");
	square_val = Array( board_size ).fill(0);
	square_val.map(_=>Array( board_size ).fill(-1));
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
	console.log(receive);
	for( let i = 0; i < board_size; ++i ) {
		for( let j = 0; j < board_size; ++j ) {
			square_div[i][j].className = "square";
			let inner = square_div[i][j].firstElementChild;
			if( receive[i][j] != -1 ) {
				inner.className = "inner_circle t"+receive[i][j];
			} else {
				inner.className = "invisible";
			}
			if( receive[i][j] != square_val[i][j] )
				square_div[i][j].classList.add("last_play");
		}
	}
	square_val = receive;
}

function startTurn( col ) {
	console.log("Your turn!");
	turn++;
	document.getElementById("turn_display").innerHTML = "Turn: " + turn;
	chaos_col = col;
	document.getElementById("player_color").className = "inner_circle t" + col;
	has_turn = true;
}

function clickSquare( r, c ) {
	if( has_turn ) {
		if( square_val[r][c] != -1 ) {
			if( square_val[r][c].classList.contains("select") ) {
				select = [-1,-1];
				square_div[r][c].classList.remove("select");
			} else {
				square_div[r][c].classList.add("select");
				select = [r,c];
			}
		}
	}
}

function loadBoard() {
	let container = document.getElementById("board_container");
	square_div = Array( board_size );
	for( let i = 0; i < board_size; ++i ) {
		let row_name = "r" + i;
		let row = document.createElement("div");
		row.id = row_name;
		row.className = "row";
		container.appendChild(row);
		row = document.getElementById(row_name);
		square_div[i] = Array( board_size );
		for( let j = 0; j < board_size; ++j ) {
			let square_name = ( "s" + i ) + j;
			let square = document.createElement("div");
			square.innerHTML = "<div class='inner_circle invisible'></div>";
			square.id = square_name;
			square.className = "square";
			square.onclick = () => clickSquare(i,j); 
			row.appendChild(square);
			square_div[i][j] = document.getElementById(square_name);
		}
	}
}



window.onload = function() {
	socket.on("gamestart",startGame);
	socket.on("board",receiveBoard);
	socket.on("disconnect", ()=>{location.href = location.href;});
	socket.on("turn",startTurn);
	loadBoard();
}
