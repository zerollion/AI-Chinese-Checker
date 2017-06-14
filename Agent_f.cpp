// Tan Zhen 872692777
// COMP 3703
// 5/25/2015

// final compete ready program

#include "Agent.h"

#include <cstdlib>
#include <unordered_map>
#include <iostream>
#include <iterator>
#include <sstream>
#include <string>
#include <vector>
#include <ctime>

// opening book approach
static const std::array<std::array<Move, 4>, 4> open = {
	std::array < Move, 4 > {{ { 27, 28 }, { 9, 29 }, { 2, 38 }, { 38, 39 } }}, //player 1-1
	{ Move{ 18, 20 }, { 20, 29 }, { 2, 38 }, { 38, 39 } },					   //player 1-2

	{ Move{ 53, 52 }, { 71, 51 }, { 78, 42 }, { 42, 41 } },					   //player 2-1
	{ Move{ 62, 60 }, { 60, 51 }, { 78, 42 }, { 42, 41 } }					   //player 2-2

};

//================== history heuristic =========================
static std::array<std::array<int, 81>, 81> history;
struct
{
	bool operator()(Move m1, Move m2){
		return history[m1.from][m1.to] > history[m2.from][m2.to];
	}
}compare;
//==============================================================


Agent::Agent() : name("AshBringer"), my_player(player1){
	state.initializehash();

	//openbook
	movecount = 0;
	rand = (uint16_t)std::rand();

	//history test
	for (int i = 0; i < 81; ++i){
		for (int j = 0; j < 81; ++j){
			history[i][j] = 0;
		}
	}
}

// pre-calculated distance function
static const float player1_dis[] = { 22.63, 21.07, 19.57, 18.16, 16.85, 15.66,
14.62, 13.76, 13.10, 21.07, 19.47, 17.94, 16.49, 15.14, 13.92, 12.86, 11.98,
11.34, 19.57, 17.94, 16.37, 14.88, 13.48, 12.22, 11.12, 10.23, 9.61, 18.16,
16.49, 14.88, 13.34, 11.89, 10.57, 9.42, 8.51, 7.90, 16.85, 15.14, 13.48,
11.89, 10.37, 8.98, 7.77, 6.81, 6.22, 15.66, 13.92, 12.22, 10.57, 8.98, 7.51,
6.20, 5.16, 4.57, 14.62, 12.86, 11.12, 9.42, 7.77, 6.20, 4.76, 3.58, 2.96,
13.76, 11.98, 10.23, 8.51, 6.81, 5.16, 3.58, 2.18, 1.41, 13.10, 11.34, 9.61,
7.90, 6.22, 4.57, 2.96, 1.41, 0.00 };
static const float player2_dis[] = { 0.00, 1.41, 2.96, 4.57, 6.22, 7.90, 9.61,
11.34, 13.10, 1.41, 2.18, 3.58, 5.16, 6.81, 8.51, 10.23, 11.98, 13.76, 2.96,
3.58, 4.76, 6.20, 7.77, 9.42, 11.12, 12.86, 14.62, 4.57, 5.16, 6.20, 7.51,
8.98, 10.57, 12.22, 13.92, 15.66, 6.22, 6.81, 7.77, 8.98, 10.37, 11.89, 13.48,
15.14, 16.85, 7.90, 8.51, 9.42, 10.57, 11.89, 13.34, 14.88, 16.49, 18.16,
9.61, 10.23, 11.12, 12.22, 13.48, 14.88, 16.37, 17.94, 19.57, 11.34, 11.98,
12.86, 13.92, 15.14, 16.49, 17.94, 19.47, 21.07, 13.10, 13.76, 14.62, 15.66,
16.85, 18.16, 19.57, 21.07, 22.63 };

static const bool hashtest = false;

Move Agent::nextMove() {
	//=================== check current player =================
	if (movecount == 0){
		openbook = open[2 * my_player + (rand % 2)];
	}

	//=================== open book approach ===================
	++movecount;
	if (movecount <= 4){
		return openbook[movecount - 1];
	}

	// Somehow select your next move
	std::vector<Move> moves;
	state.getMoves(moves);

	// eliminate backward moves
	std::vector<Move>::iterator i;
	for (i = moves.begin(); i != moves.end();){
		if (!isbackward(my_player, *i)) i = moves.erase(i);
		else ++i;
	}
	std::random_shuffle(moves.begin(), moves.end());

	// sort based on heuristic
	std::sort(moves.begin(), moves.end(), compare);
	//==============================================================

	double evl = evaluation(state.getboard(), { 0, 0 }, 0);
	double alpha = -10000.0;
	double beta = 10000.0;
	int depth = 1;

	unsigned maxIndex = 0;
	unsigned testIndex = 0;
	uint64_t hashvalue = 0;
	std::array<int, 81> board = state.getboard();
	clock_t startTime = clock();

	//================ new tranpertation table ==================
	std::unordered_map<uint64_t, states> tt;
	tt.clear();

	// root node is max node, the first recursive call should be min node
	// need to record index
	for (int i = 0; i < moves.size(); i++){

		//if (!isbackward(current_player, moves[i])) continue;

		double e = evl;
		state.applyMove(moves[i]);
		board = state.getboard();

		//================ tranpertation table operation ==================
		hashvalue = state.gethash();
		auto it = tt.find(hashvalue);
		if (it != tt.end() && it->second.depth >= depth &&
			it->second.alpha > alpha && it->second.beta < beta){
			states hashstate = it->second;
			e = hashstate.value;
			alpha = hashstate.alpha;
			beta = hashstate.beta;

			std::cerr << "TT hit" << std::endl;
		}
		else{
			std::vector<Move> newmoves;
			state.getMoves(newmoves);
			std::random_shuffle(newmoves.begin(), newmoves.end());
			e = minNode(board, newmoves, depth, startTime, alpha, beta, tt, moves[i], evl);

			states s = states(depth, alpha, beta, e);
			tt.emplace(state.gethash(), s);
		}

		if (e > evl){
			maxIndex = i;
			evl = e;
		}
		//================ test part ======================================
		if (hashtest == true){
			Move max = moves[maxIndex];
			std::vector<Move> newmoves;
			state.getMoves(newmoves);
			std::random_shuffle(newmoves.begin(), newmoves.end());
			e = minNode(board, newmoves, depth, startTime, alpha, beta, tt, moves[i], evl);
			if (e > evl){
				testIndex = i;
				evl = e;
			}
			Move test = moves[testIndex];
			std::cerr << "Test result, transportaion table give the same move? "
				<< (test == max) << std::endl;
		}
		//=================================================================
		state.undoMove(moves[i]);

		alpha = std::max(alpha, evl);
		if (beta <= alpha) {
			//================ history heuristic ===================
			history[moves[i].from][moves[i].to] += depth;
			return moves[maxIndex];
		}
		clock_t currentTime = clock();
		if (((currentTime - startTime) / (double)CLOCKS_PER_SEC) > 9.5){
			return moves[maxIndex];
		}
	}

	//=============================================================================================================
	//history test
	std::cerr << "current Move count:" << movecount << std::endl;
	/*for (int i = 0; i < 81; ++i){
		for (int j = 0; j < 81; ++j){
			std::cerr << history[i][j] << "," << i << "," << j << "...";
		}
		std::cerr << std::endl;
	}*/
	//=============================================================================================================

	return moves[maxIndex];

}

//===============================================
// evaluation function
double Agent::evaluation(std::array<int, 81> board, Move m, double old_value){

	double player1 = 0;
	double player2 = 0;
	double val;

	// new distance function with pre-calculated distance
	if (old_value == 0 || m == Move{ 0, 0 }){
		for (int i = 0; i < 81; i++){
			if (board[i] == 1){
				player1 += player1_dis[i];
			}
			else if (board[i] == 2){
				player2 += player2_dis[i];
			}
		}
		if (current_player == 0){
			return player2 - player1;
		}
		else{
			return player1 - player2;
		}
	}

	// for future use, simlifying the evaluation function
	if (my_player == 0){
		val = old_value + player1_dis[m.from] - player1_dis[m.to];
	}
	else if (my_player == 1){
		val = old_value + player2_dis[m.from] - player2_dis[m.to];
	}


	return val;
}

/*double Agent::evaluation(std::array<int, 81> board, Move m, double old_value){

	double player1 = 0;
	double player2 = 0;
	double val = 0;
	for (int i = 0; i < 81; i++){
		if (board[i] == 1){
			player1 += player1_dis[i];
		}
		else if (board[i] == 2){
			player2 += player2_dis[i];
		}
	}
	if (current_player == 0){
		return player2 - player1;
	}
	else{
		return player1 - player2;
	}
}*/

//===============================================
// max node function
double Agent::maxNode(std::array<int, 81> board, std::vector<Move> moves,
	int depth, clock_t startTime, double alpha, double beta,
	std::unordered_map<uint64_t, states> tt,
	Move currentMove, double currentValue){

	clock_t currentTime = clock();
	if (depth == 0 || ((currentTime - startTime) / (double)CLOCKS_PER_SEC) > 9.0){
		return evaluation(board, currentMove, currentValue);
	}
	else if (state.gameOver()){
		return -10000.0;
	}

	double newevl = -10000.0;
	for (int i = 0; i < moves.size(); i++){
		// eleiminate backward moves
		if (!isbackward(current_player, moves[i])) continue;

		state.applyMove(moves[i]);

		double e = -10000.0;
		uint64_t hashvalue = state.gethash();
		auto it = tt.find(hashvalue);
		if (it != tt.end() && it->second.depth >= depth &&
			it->second.alpha > alpha && it->second.beta < beta){
			states hashstate = it->second;
			e = hashstate.value;
			alpha = hashstate.alpha;
			beta = hashstate.beta;
			//std::cerr << "hit in max node" << std::endl;
		}
		else{
			std::vector<Move> newmoves;
			state.getMoves(newmoves);
			std::random_shuffle(newmoves.begin(), newmoves.end());

			e = minNode(board, newmoves, depth, startTime, beta - 1, beta, tt, moves[i], currentValue);
			newevl = std::max(newevl, e);
			alpha = std::max(newevl, alpha);

			states s = states(depth, alpha, beta, e);
			tt.emplace(state.gethash(), s);
		}

		state.undoMove(moves[i]);
		//==================== alpha-beta prunning =================
		if (beta <= alpha){
			//================ history heuristic ===================
			history[currentMove.from][currentMove.to] += depth;
			return newevl;
		}

	}

	return newevl;
}
//===============================================
// min node function
double Agent::minNode(std::array<int, 81> board, std::vector<Move> moves,
	int depth, clock_t startTime, double alpha, double beta,
	std::unordered_map<uint64_t, states> tt,
	Move currentMove, double currentValue){

	clock_t currentTime = clock();
	if (depth == 0 || ((currentTime - startTime) / (double)CLOCKS_PER_SEC) > 9.0){
		return evaluation(board, currentMove, currentValue);
	}
	else if (state.gameOver()){
		return 10000.0;
	}

	double newevl = 10000.0;
	for (int i = 0; i < moves.size(); i++){
		// eleiminate backward moves
		if (!isbackward(current_player, moves[i])) continue;

		state.applyMove(moves[i]);

		double e = 10000.0;
		uint64_t hashvalue = state.gethash();
		auto it = tt.find(hashvalue);
		if (it != tt.end() && it->second.depth >= depth &&
			it->second.alpha > alpha && it->second.beta < beta){
			states hashstate = it->second;
			e = hashstate.value;
			alpha = hashstate.alpha;
			beta = hashstate.beta;
			//std::cerr << "hit in min node" << std::endl;
		}
		else{
			std::vector<Move> newmoves;
			state.getMoves(newmoves);
			std::random_shuffle(newmoves.begin(), newmoves.end());

			e = maxNode(board, newmoves, depth, startTime, beta - 1, beta, tt, currentMove, currentValue);
			newevl = std::min(newevl, e);
			beta = std::min(newevl, beta);

			states s = states(depth, alpha, beta, e);
			tt.emplace(state.gethash(), s);
		}

		state.undoMove(moves[i]);

		//==================== alpha-beta prunning =================
		if (beta <= alpha){
			//================ history heuristic ===================
			history[currentMove.from][currentMove.to] += depth;
			return newevl;
		}
	}

	return newevl;
}
//===============================================

// return true if the move is a backward move
bool Agent::isbackward(Players current, Move m){

	int f = m.from % 9 + m.from / 9;
	int t = m.to % 9 + m.to / 9;
	if (current == 0){
		// player 1
		return t >= f;
	}

	return t <= f;
}

void Agent::playGame() {
	// Identify myself
	std::cout << "#name " << name << std::endl;

	// Wait for start of game
	waitForStart();

	// Main game loop
	for (;;) {
		if (current_player == my_player) {
			// My turn

			// Check if game is over
			if (state.gameOver()) {
				std::cerr << "I, " << name << ", have lost" << std::endl;
				switchCurrentPlayer();
				continue;
			}

			// Determine next move
			const Move m = nextMove();

			// Apply it locally
			state.applyMove(m);

			// Tell the world
			printAndRecvEcho(m);

			// It is the opponents turn
			switchCurrentPlayer();
		}
		else {
			// Wait for move from other player
			// Get server's next instruction
			std::string server_msg = readMsg();
			const std::vector<std::string> tokens = tokenizeMsg(server_msg);

			if (tokens.size() == 5 && tokens[0] == "MOVE") {
				// Translate to local coordinates and update our local state
				const Move m = state.translateToLocal(tokens);
				state.applyMove(m);

				// It is now my turn
				switchCurrentPlayer();
			}
			else if (tokens.size() == 4 && tokens[0] == "FINAL" &&
				tokens[2] == "BEATS") {
				// Game over
				if (tokens[1] == name && tokens[3] == opp_name)
					std::cerr << "I, " << name << ", have won!" << std::endl;
				else if (tokens[3] == name && tokens[1] == opp_name)
					std::cerr << "I, " << name << ", have lost." << std::endl;
				else
					std::cerr << "Did not find expected players in FINAL command.\n"
					<< "Found '" << tokens[1] << "' and '" << tokens[3] << "'. "
					<< "Expected '" << name << "' and '" << opp_name << "'.\n"
					<< "Received message '" << server_msg << "'" << std::endl;
				break;
			}
			else {
				// Unknown command
				std::cerr << "Unknown command of '" << server_msg << "' from the server"
					<< std::endl;
			}
		}
	}
}

// Sends a msg to stdout and verifies that the next message to come in is it
// echoed back. This is how the server validates moves
void Agent::printAndRecvEcho(const std::string &msg) const {
	// Note the endl flushes the stream, which is necessary
	std::cout << msg << std::endl;
	const std::string echo_recv = readMsg();
	if (msg != echo_recv)
		std::cerr << "Expected echo of '" << msg << "'. Received '" << echo_recv
		<< "'" << std::endl;
}

// Reads a line, up to a newline from the server
std::string Agent::readMsg() const {
	std::string msg;
	std::getline(std::cin, msg); // This is a blocking read

	// Trim white space from beginning of string
	const char *WhiteSpace = " \t\n\r\f\v";
	msg.erase(0, msg.find_first_not_of(WhiteSpace));
	// Trim white space from end of string
	msg.erase(msg.find_last_not_of(WhiteSpace) + 1);

	return msg;
}

// Tokenizes a message based upon whitespace
std::vector<std::string> Agent::tokenizeMsg(const std::string &msg) const {
	// Tokenize using whitespace as a delimiter
	std::stringstream ss(msg);
	std::istream_iterator<std::string> begin(ss);
	std::istream_iterator<std::string> end;
	std::vector<std::string> tokens(begin, end);

	return tokens;
}

void Agent::waitForStart() {
	for (;;) {
		std::string response = readMsg();
		std::vector<std::string> tokens = tokenizeMsg(response);

		if (tokens.size() == 4 && tokens[0] == "BEGIN" &&
			tokens[1] == "CHINESECHECKERS") {
			// Found BEGIN GAME message, determine if we play first
			if (tokens[2] == name) {
				// We go first!
				opp_name = tokens[3];
				my_player = player1;
				break;
			}
			else if (tokens[3] == name) {
				// They go first
				opp_name = tokens[2];
				my_player = player2;
				break;
			}
			else {
				std::cerr << "Did not find '" << name
					<< "', my name, in the BEGIN command.\n"
					<< "# Found '" << tokens[2] << "' and '" << tokens[3] << "'"
					<< " as player names. Received message '" << response << "'"
					<< std::endl;
				std::cout << "#quit" << std::endl;
				std::exit(EXIT_FAILURE);
			}
		}
		else if (response == "DUMPSTATE") {
			std::cout << state.dumpState() << std::endl;
		}
		else if (tokens[0] == "LOADSTATE") {
			std::string new_state = response.substr(10);
			if (!state.loadState(new_state))
				std::cerr << "Failed to load '" << new_state << "'\n";
		}
		else if (response == "LISTMOVES") {
			std::vector<Move> moves;
			state.getMoves(moves);
			for (const auto i : moves)
				std::cout << i.from << ", " << i.to << "; ";
			std::cout << std::endl;
		}
		else if (tokens[0] == "MOVE") {
			// Just apply the move
			const Move m = state.translateToLocal(tokens);
			if (!state.applyMove(m))
				std::cout << "Unable to apply move " << m << std::endl;
		}
		else if (tokens[0] == "UNDO") {
			tokens[0] = "MOVE";
			const Move m = state.translateToLocal(tokens);
			if (!state.undoMove(m))
				std::cout << "Unable to undo move " << m << std::endl;
		}
		else if (response == "NEXTMOVE") {
			const Move m = nextMove();
			std::cout << m.from << ", " << m.to << std::endl;
		}
		else if (response == "EVAL") {
			double evl = evaluation(state.getboard(), Move{ 0, 0 }, 0);
			//double evl = evaluation(state.getboard());
			std::cout << evl << std::endl;
		}
		else {
			std::cerr << "Unexpected message " << response << "\n";
		}
	}

	// Game is about to begin, restore to start state in case DUMPSTATE/LOADSTATE/LISTMOVES
	// were used
	state.reset();

	// Player 1 goes first
	current_player = player1;
}

void Agent::switchCurrentPlayer() {
	current_player = (current_player == player1) ? player2 : player1;
}

bool Agent::isValidStartGameMessage(const std::vector<std::string> &tokens) const {
	return tokens.size() == 4 && tokens[0] == "BEGIN" && tokens[1] == "CHINESECHECKERS";
}

bool Agent::isValidMoveMessage(const std::vector<std::string> &tokens) const {
	return tokens.size() == 5 && tokens[0] == "MOVE" && tokens[1] == "FROM" &&
		tokens[3] == "TO";
}
