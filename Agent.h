//===------------------------------------------------------------*- C++ -*-===//
#ifndef AGENT_H_INCLUDED
#define AGENT_H_INCLUDED

#include <string>
#include <vector>
#include <ctime>
#include <unordered_map>

#include "ChineseCheckersState.h"

// data sets for hashing
struct states{
	int depth;
	double alpha;
	double beta;
	double value;

	states(int d, double a, double b, double v){
		depth = d;
		alpha = a;
		beta = b;
		value = v;
	}
};

class Agent {
public:
	Agent();
	void playGame();

private:
	Move nextMove();
	void printAndRecvEcho(const std::string &msg) const;
	std::string readMsg() const;
	std::vector<std::string> tokenizeMsg(const std::string &msg) const;
	void waitForStart();
	void switchCurrentPlayer();

	bool isValidStartGameMessage(const std::vector<std::string> &tokens) const;
	bool isValidMoveMessage(const std::vector<std::string> &tokens) const;

	ChineseCheckersState state;
	enum Players { player1, player2 };
	Players current_player;
	Players my_player;
	std::string name;
	std::string opp_name;

	//============================================
	//double evaluation(std::array<int, 81> board);
	double maxNode(std::array<int, 81> board, std::vector<Move> moves,
		int depth, clock_t time, double alpha, double beta,
		std::unordered_map<uint64_t, states> tt,
		Move currentMove, double currentValue);
	double minNode(std::array<int, 81> board, std::vector<Move> moves,
		int depth, clock_t time, double alpha, double beta,
		std::unordered_map<uint64_t, states> tt,
		Move currentMove, double currentValue);
	// new evaluation function
	double evaluation(std::array<int, 81> board, Move m, double old_value);
	// decide whether the move is a backward move, return true if the move is a backward move
	bool isbackward(Players current_player, Move m);
	//============================================
	// openbook approach
	int movecount;
	uint16_t rand;
	std::array<Move, 4> openbook;
	//============================================


};


#endif
