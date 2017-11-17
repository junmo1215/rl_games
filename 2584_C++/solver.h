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
		expects = new float[SIZE];
		after_expects = new float[SIZE];
		for(long i = 0; i < SIZE; i++){
			// moves[i] = -1;
			expects[i] = -1.0;
			after_expects[i] = -1.0;
		}

		for(int pos1 = 0; pos1 < max_pos; pos1++){
			for(int tile1 = 1; tile1 < 3; tile1++){

				for(int pos2 = pos1 + 1; pos2 < max_pos; pos2++){
					for(int tile2 = 1; tile2 < 3; tile2++){
						board2x3 board;
						action::place(tile1, pos1).apply(board);
						action::place(tile2, pos2).apply(board);
						get_before_expect(board);
					}
				}
			}
		}

		std::cout << "solver is initialized." << std::endl << std::endl;
	}

	answer solve2x3(const board2x3& state, state_type type = state_type::before) {
		int temp_tile;
		for(int i = 0; i < row; i++){
			for(int j = 0; j < column; j++){
				temp_tile = state[i][j];
				// 排除盘面上不可能出现的数字
				if(temp_tile >= max_index || temp_tile < 0)
					return -1;
			}
		}

		if(type.is_before() && is_legal_before_state(state)){
			return get_before_expect(state);
		}
		else if(type.is_after() && is_legal_after_state(state)){
			return get_after_expect(state);
		}
		return -1;
	}

private:
	float *expects;
	float *after_expects;

private:

	float get_before_expect(board2x3 board){
		int index = get_index(board);

		if(expects[index] > -1)
			return expects[index];

		float expect = 0.0;
		float best_expect = MIN_FLOAT;
		bool is_moved = false;

		// 模拟四种动作
		for(int op: {3, 2, 1, 0}){
			board2x3 b = board;
			int reward = b.move(op);
			if(reward != -1){
				expect = get_after_expect(b) + reward;
				if(expect > best_expect){
					is_moved = true;
					best_expect = expect;
				}
			}
		}

		if(is_moved){
			expects[index] = best_expect;
		}
		else{
			expects[index] = 0;
		}

		return expects[index];
	}

	float get_after_expect(board2x3 board){
		float temp_expects[2] = {0, 0};
		for(int i: {1, 2}){
			float expect = 0;
			int count = 0;
			for(int j = 0; j < max_pos; j++){
				board2x3 b = board;
				int result = action::place(i, j).apply(b);
				if(result != -1){
					expect += get_before_expect(b);
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
	}

	bool is_legal_before_state(board2x3 board){
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
