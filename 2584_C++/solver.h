#pragma once
#include <iostream>
#include <string>
#include <cmath>
#include "board2x3.h"
#include "type.h"

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
		std::cout << SIZE << std::endl;
		int best_move[SIZE];
		float expect[SIZE];
		
		for(long long i = 0; i < SIZE; i++){
			best_move[i] = -1;
			expect[i] = -1.0;
		}

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

	int temp = 14;
	const long long SIZE = temp * temp * temp * temp * temp * temp;


	// TODO: place your transposition table here
};
