/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++0x -O3 -g -Wall -fmessage-length=0 -o 2048sol 2048sol.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <iomanip>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <random>
#include <sstream>
#include <fstream>
#include <cmath>
#include <chrono>

#include "action2x3.h"
#include "board2x3.h"
#include "type.h"
#include "solver.h"

int main(int argc, const char* argv[]) {
	std::cout << "2048-Solver: ";
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	std::string solve_args;
	int precision = 10;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--solve=") == 0) {
			solve_args = para.substr(para.find("=") + 1);
		} else if (para.find("--precision=") == 0) {
			precision = std::stol(para.substr(para.find("=") + 1));
		}
	}

	solver solve(solve_args);
	board2x3 state;
	state_type type;
	std::cout << std::setprecision(precision);
	while (std::cin >> type >> state) {
		auto ans = solve.solve2x3(state, type);
		std::cout << "= " << ans << std::endl;
	}

	return 0;
}
