/**
 * Basic Environment for Game 2584
 * use 'g++ -std=c++0x -O3 -g -o 2584 2584.cpp' to compile the source
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
#include "bitboard.h"
#include "action.h"
#include "agent.h"
#include "statistic.h"
#include <cstdio>
#include <cstring>
#include<time.h>

/**
* 这里只记录了最近1000局(limit)的盘面信息以及统计结果，没有保存weight
*/
void save_statistic(const statistic stat, const std::string save){
	if (save.size()) {
		std::ofstream out;
		out.open(save.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) return;
		out << stat;
		out.flush();
		out.close();
	}
}

int main(int argc, const char* argv[]) {
	clock_t startTime,endTime;
	startTime = clock();
    //freopen("2.txt","w",stdout);

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
		play.open_episode("~:" + evil.name());
		evil.open_episode(play.name() + ":~");

		stat.open_episode(play.name() + ":" + evil.name());
		board game = stat.make_empty_board();

        while (true) {
            agent& who = stat.take_turns(play, evil);
            action move = who.take_action(game);
            if (move.apply(game) == -1) break;
            stat.save_action(move);
            if (who.check_for_win(game)) break;
        }
        agent& win = stat.last_turns(play, evil);
        stat.close_episode(win.name());

        play.close_episode(win.name());
        evil.close_episode(win.name());

		// 每10000局保存一次统计信息
        if(i % 10000 == 0)
			save_statistic(stat, save);
		
		// 每200000局改变一次learning rate
		if(i % 200000 == 0)
			play.change_learning_rate();

		// 每50000局保存一次weights
		if(i % 50000 == 0){
			play.save_weights();
		}
    }

	if (summary) {
		stat.summary();
	}

	save_statistic(stat, save);

	endTime = clock();
	std::cout << "Totle Time : " <<(double)(endTime - startTime) / CLOCKS_PER_SEC << "s" << std::endl;
	return 0;
}
