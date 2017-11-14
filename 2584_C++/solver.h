#pragma once
#include <iostream>
#include <string>
#include <cmath>
#include "board2x3.h"
#include "type.h"
#include "action.h"

class solver {
public:
	typedef float value_t;

public:
	class answer {
	public:
		answer(value_t value) : value(value) {}
	    friend std::ostream& operator <<(std::ostream& out, const answer& ans) {
	    	return out << (std::isnan(ans.value) ? -1 : ans.value);
		}
	public:
		const value_t value;
	};

public:
	solver(const std::string& args) {
		// int best_move[SIZE];
		// float expect[SIZE];

		for(long long i = 0; i < SIZE; i++){
			moves[i] = -1;
			expects[i] = -1.0;
		}

		// int i = 0;

		for(int pos1 = 0; pos1 < max_pos; pos1++){
			for(int tile1 = 1; tile1 < 3; tile1++){

				for(int pos2 = 0; pos2 < max_pos; pos2++){
					for(int tile2 = 1; tile2 < 3; tile2++){
						/* 
						* 去掉重複的以及不合法的放置位置
						* 如果写成等于会导致以下两种过程造成的结果相同
						* 1. place(1, 1) place(1, 2)
						* 2. place(1, 2) place(1, 1)
						*/
						if(pos1 >= pos2)
							continue;
						// i++;
						board2x3 board;
						action::place(tile1, pos1).apply(board);
						action::place(tile2, pos2).apply(board);
						// std::cout << board << std::endl;
						get_expect(board);
					}
				}
			}
		}

		// std::cout << i << std::endl;

		// printf(
		// 	"%d, %d, %d, %d, %d \n",
		// 	best_move[0], best_move[1],
		// 	best_move[5], best_move[100],
		// 	best_move[SIZE-1]);

		// printf(
		// 	"%f, %f, %f, %f, %f \n",
		// 	expect[0], expect[1],
		// 	expect[5], expect[100],
		// 	expect[SIZE-1]);


		// for(int i = 0; i < board2x3::ROW; i++){
		// 	for(int j = 0; j < board2x3::COLUMN; j++){

		// 	}
		// }

		// TODO: explore the tree and save the result
		// std::cout << "feel free to display some messages..." << std::endl;
		std::cout << "solver is initialized." << std::endl << std::endl;
	}

	answer solve2x3(const board2x3& state, state_type type = state_type::before) {
		std::cout << state << std::endl;
		// TODO: find the answer in the lookup table and return it
		return -1;
	}

private:

	int max_index = board2x3::POSSIBLE_INDEX;
	int row = board2x3::ROW;
	int column = board2x3::COLUMN;
	int max_pos = row * column;
	const long SIZE = max_index * max_index * max_index * max_index * max_index * max_index;
	int *moves = new int[SIZE];
	float *expects = new float[SIZE];

	// TODO: place your transposition table here

private:

	float get_expect(board2x3 board){
		// std::cout << board << std::endl;
		int index = 0;
		for(int i = 0; i < row; i++){
			for(int j = 0; j < column; j++){
				index *= max_index;
				index += board[i][j];
			}
		}

		if(expects[index] > -1)
			return expects[index];

		// std::cout << board << std::endl << index << std::endl;
		// std::cin.ignore();

		float expect = 0.0;
		float best_expect = MIN_FLOAT;
		int best_move = 0;
		bool is_moved = false;

		// 模拟四种动作
		for(int op: {3, 2, 1, 0}){
			board2x3 b = board;
			int reward = b.move(op);
			// std::cout << "op: " << op << std::endl;
			// std::cout << "reward: " << reward << std::endl;
			// std::cout << b;
			if(reward != -1){
				expect = get_after_expect(b) + reward;
				if(expect > best_expect){
					is_moved = true;
					best_expect = expect;
					best_move = op;
				}
			}
		}

		if(is_moved){
			expects[index] = best_expect;
			moves[index] = best_move;
		}
		else{
			expects[index] = 0;
		}
		return expects[index];
	}

	float get_after_expect(board2x3 board){
		// std::cout << board << std::endl;
		float expect = 0.0;
		float best_expect = MIN_FLOAT;
		
		for(int i = 0; i < max_pos; i++){
			for(int j = 1; j < 3; j++){
				board2x3 b = board;
				int result = action::place(j, i).apply(b);
				if(result != -1){
					// std::cout << b << std::endl;
					expect = 0 - get_expect(b);
					if(expect > best_expect){
						best_expect = expect;
					}
				}
			}
		}
		return best_expect;
	}

};
