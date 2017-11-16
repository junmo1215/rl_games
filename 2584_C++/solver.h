#pragma once
#include <iostream>
#include <string>
#include <cmath>
#include "board2x3.h"
#include "type.h"
#include "action.h"

static const int max_index = board2x3::POSSIBLE_INDEX;
static const int row = board2x3::ROW;
static const int column = board2x3::COLUMN;
static const int max_pos = row * column;
static const long SIZE = max_index * max_index * max_index * max_index * max_index * max_index;

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

		expects = new float[SIZE];
		after_expects = new float[SIZE];
		for(long i = 0; i < SIZE; i++){
			// moves[i] = -1;
			expects[i] = -1.0;
			after_expects[i] = -1.0;
		}

		// std::cout << "expects values inited" << std::endl;
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

		// test
		// board2x3 board;
		// action::place(6, 0).apply(board);
		// action::place(2, 1).apply(board);
		// action::place(1, 2).apply(board);
		// action::place(8, 3).apply(board);
		// action::place(10, 4).apply(board);
		// action::place(12, 5).apply(board);

		// //std::cout << SIZE << std::endl;
		// //std::cout << expects[3451066] << std::endl;
		// get_expect(board);
		
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
		// std::cout << state << std::endl;

		// int sum = 0;
		int temp_tile;
		for(int i = 0; i < row; i++){
			for(int j = 0; j < column; j++){
				temp_tile = state[i][j];
				// 排除盘面上不可能出现的数字
				if(temp_tile >= max_index || temp_tile < 0)
					return -1;
				// sum += temp_tile;
			}
		}
		// // 排除盘面上只有一个1或全部为空的情况
		// if(sum <= 1)
		// 	return -1;

		// TODO: find the answer in the lookup table and return it
		if(type.is_before() && is_legal_before_state(state)){
			// std::cout << "is_before" << std::endl;
			// std::cout << state << std::endl;
			return get_expect(state);
		}
		else if(type.is_after() && is_legal_after_state(state)){
			// std::cout << "is_after" << std::endl;
			// std::cout << state << std::endl;
			return get_after_expect(state);
		}
		return -1;
	}

private:
	// int *moves = new int[SIZE];
	float *expects;
	float *after_expects;

	// TODO: place your transposition table here

private:

	float get_expect(board2x3 board){
		// std::cout << board << std::endl;
		int index = get_index(board);
		// for(int i = 0; i < row; i++){
		// 	for(int j = 0; j < column; j++){
		// 		index *= max_index;
		// 		index += board[i][j];
		// 	}
		// }

		// std::cout << index << std::endl;
		// std::cout << board << std::endl;
		if(expects[index] > -1)
			return expects[index];

		// std::cout << index << "\t" << expects[index] << std::endl;
		// std::cout << board << std::endl << index << std::endl;
		// std::cin.ignore();

		float expect = 0.0;
		float best_expect = MIN_FLOAT;
		// int best_move = 0;
		bool is_moved = false;

		// 模拟四种动作
		for(int op: {3, 2, 1, 0}){
			board2x3 b = board;
			int reward = b.move(op);
			// std::cout << "op: " << op << std::endl;
			// std::cout << "reward: " << reward << std::endl;
			// std::cout << b;
			// std::cin.ignore();
			if(reward != -1){
				expect = get_after_expect(b) + reward;
				if(expect > best_expect){
					is_moved = true;
					best_expect = expect;
					// best_move = op;
				}
			}
		}

		if(is_moved){
			expects[index] = best_expect;
			// moves[index] = best_move;
		}
		else{
			expects[index] = 0;
		}

		return expects[index];
	}

	float get_after_expect(board2x3 board){
		// std::cout << board << std::endl;
		// float expect = 0.0;
		// float best_expect = 0 - MIN_FLOAT;
		//int count[2] = {0, 0};

		// for(int i = 0; i < max_pos; i++){
		// 	for(int j: {1, 2}){
		// 		board2x3 b = board;
		// 		int result = action::place(j, i).apply(b);
		// 		if(result != -1){
		// 			count[j - 1] += 1;
		// 			// std::cout << b << std::endl;
		// 			expect += get_expect(b);
		// 			// if(expect < best_expect){
		// 			// 	best_expect = expect;
		// 			// }
		// 		}
		// 	}
		// }

		float temp_expects[2] = {0, 0};
		for(int i: {1, 2}){
			float expect = 0;
			int count = 0;
			for(int j = 0; j < max_pos; j++){
				board2x3 b = board;
				int result = action::place(i, j).apply(b);
				if(result != -1){
					expect += get_expect(b);
					count++;
				}
			}
			temp_expects[i - 1] = expect / count;
		}

		float result = temp_expects[0] * 0.9 + temp_expects[1] * 0.1;
		after_expects[get_index(board)] = result;

		return result;
	}

	bool is_legal_after_state(board2x3 board){

		int index = get_index(board);
		return after_expects[index] != -1;
		// int temp_tile;
		// for(int i = 0; i < row; i++){
		// 	for(int j = 0; j < column; j++){
		// 		temp_tile = board[i][j];
		// 		if
		// 	}
		// }



		// 玩家移动后盘面上必然会出现空位
		// 空位太多的某些情况也不可能出现
		// 这时候随便放一个tile一定是before state
		// 用上面两个逻辑排除了除了初始条件之外的各种情况
		// int empty_count = 0;
		// int one_count = 0;
		// int temp_tile;
		// for(int i = 0; i < row; i++){
		// 	for(int j = 0; j < column; j++){
		// 		temp_tile = board[i][j];
		// 		if(temp_tile == 0){
		// 			if(empty_count == 0){
		// 				board2x3 b = board;
		// 				action::place(1, i * 3 + j).apply(b);
		// 				if(is_legal_before_state(b) == false)
		// 					return false;
		// 			}
		// 			empty_count++;
		// 		}
		// 		else if(temp_tile == 1){
		// 			one_count++;
		// 		}
					
		// 	}
		// }
		// if(empty_count == 0)
		// 	return false;

		// // 不可能在移动之后盘面上只有一个1
		// if(empty_count == 5 && one_count == 1)
		// 	return false;

		// // 考虑类似 1 0 0 0 0 1这样的情况


		// return true;
	}

	bool is_legal_before_state(board2x3 board){
		// // 玩家移动前，盘面上必然会有1或者2
		// int drop_tile_count = 0;
		// int temp_tile;
		// for(int i = 0; i < row; i++){
		// 	for(int j = 0; j < column; j++){
		// 		temp_tile = board[i][j];
		// 		if(temp_tile == 1 or temp_tile == 2)
		// 			drop_tile_count++;
		// 	}
		// }
		// if(drop_tile_count == 0)
		// 	return false;
		// return true;

		int index = get_index(board);
		return expects[index] != -1;

	}

	int get_index(board2x3 board){
		int index = 0;
		for(int i = 0; i < row; i++){
			for(int j = 0; j < column; j++){
				index *= max_index;
				index += board[i][j];
			}
		}
		return index;
	}
};
