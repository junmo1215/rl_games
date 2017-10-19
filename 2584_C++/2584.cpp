/**
 * Basic Environment for Game 2048
 * use 'g++ -std=c++0x -O3 -g -o 2048 2048.cpp' to compile the source
 *
 * Computer Games and Intelligence (CGI) Lab, NCTU, Taiwan
 * http://www.aigames.nctu.edu.tw
 */

#include <iostream>
#include <algorithm>
#include <functional>
#include <iterator>
#include <string>
#include <random>
#include <sstream>
#include <fstream>
#include <cmath>
#include <chrono>
#include "board.h"
#include "action.h"
#include "agent.h"
#include "statistic.h"
#include <cstdio>
#include <cstring>

int main(int argc, const char* argv[]) {

    // freopen("1.txt","w",stdout);

	std::cout << "2584: ";	
	std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
	std::cout << std::endl << std::endl;

	// 设置默认参数以及解析参数
	size_t total = 1000, block = 0, limit = 0;
	std::string play_args, evil_args;
	std::string load, save;
	bool summary = false;
	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--total=") == 0) {
			total = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--block=") == 0) {
			block = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--limit=") == 0) {
			limit = std::stoull(para.substr(para.find("=") + 1));
		} else if (para.find("--play=") == 0) {
			play_args = para.substr(para.find("=") + 1);
		} else if (para.find("--evil=") == 0) {
			evil_args = para.substr(para.find("=") + 1);
		} else if (para.find("--load=") == 0) {
			load = para.substr(para.find("=") + 1);
		} else if (para.find("--save=") == 0) {
			save = para.substr(para.find("=") + 1);
		} else if (para.find("--summary") == 0) {
			summary = true;
		}
	}

	statistic stat(total, block, limit);

	// 如果设置了load位置
	if (load.size()) {
		std::ifstream in;
		// 打开文件
		in.open(load.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) return -1;
		in >> stat;
		in.close();
	}

	player play(play_args);
	rndenv evil(evil_args);

	long long i = 0;
	while (!stat.is_finished()) {
		i++;
        // std::cout << i << std::endl;
		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		board game = stat.make_empty_board();
		
        // std::cout << game << std::endl;
        // action temp;
        // // temp = std::move(action::place(2, 0));
        // (action::place(2, 0)).apply(game);
        // (action::place(12, 1)).apply(game);
        // (action::place(8, 2)).apply(game);
        // (action::place(3, 3)).apply(game);
        // (action::place(2, 4)).apply(game);
        // (action::place(5, 5)).apply(game);
        // (action::place(5, 6)).apply(game);
        // (action::place(5, 7)).apply(game);

        // (action::place(14, 8)).apply(game);
        // (action::place(11, 9)).apply(game);
        // (action::place(3, 10)).apply(game);
        // (action::place(2, 11)).apply(game);
        // (action::place(3, 12)).apply(game);
        // (action::place(9, 13)).apply(game);
        // (action::place(1, 14)).apply(game);
        // (action::place(2, 15)).apply(game);
        
        // std::cout << game << std::endl;
        // // throw;
        long long j = 0;
        bool print = false;
        while (true) {
            j++;
            agent& who = stat.take_turns(play, evil);
            // agent& who = play;
            action move = who.take_action(game, print);
            // action move;
            if(i == 177 && j >= 782 && j <= 790){
                // move = who.take_action(game, true);
                print = false;
                // std::cout << game << std::endl;
                // std::cout << move.name() << std::endl;
            }
            else{
                print = false;
                // move = who.take_action(game);
            }
            if (move.apply(game) == -1) break;
            stat.save_action(move);
            if (who.check_for_win(game)) break;
            // throw;
            // std::cout << game;
            // std::cin.ignore();
        }
        agent& win = stat.last_turns(play, evil);
        stat.close_episode(win.name());

        play.close_episode(win.name());
        evil.close_episode(win.name());

        // if(i % 1000 == 0)
        //     stat.summary();
    }

	if (summary) {
		stat.summary();
	}

	if (save.size()) {
		std::ofstream out;
		out.open(save.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) return -1;
		out << stat;
		out.flush();
		out.close();
	}

	return 0;
}