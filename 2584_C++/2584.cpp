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
#include <map>
#include <regex>
#include <memory>
#include "board.h"
// #include "bitboard.h"
#include "action.h"
#include "agent.h"
#include "statistic.h"
#include <cstdio>
#include <cstring>
#include<time.h>


int shell(int argc, const char* argv[]) {

	struct match {
		std::string id;
		std::shared_ptr<agent> play;
		std::shared_ptr<agent> evil;
		board b;
		std::shared_ptr<statistic> stat;
		match(const std::string& id, const std::string& info) : id(id) {
			play = std::shared_ptr<agent>(new agent("name=" + info.substr(0, info.find(':')) + " role=dummy"));
			evil = std::shared_ptr<agent>(new agent("name=" + info.substr(info.find(':') + 1) + " role=dummy"));
			stat = std::shared_ptr<statistic>(new statistic(1, -1));
		}
	};

	std::map<std::string, std::shared_ptr<agent>> lounge;
	std::map<std::string, std::shared_ptr<match>> arena;
	bool save_statistics = false;
	bool debug_mode = false;

	for (int i = 1; i < argc; i++) {
		std::string para(argv[i]);
		if (para.find("--play") == 0) {
			std::string args = para.find("--play=") == 0 ? para.substr(para.find("=") + 1) : "";
			std::shared_ptr<agent> who(new player(args)); // TODO: change to your player agent
			lounge[who->name()] = who;
		} else if (para.find("--evil") == 0) {
			std::string args = para.find("--evil=") == 0 ? para.substr(para.find("=") + 1) : "";
			std::shared_ptr<agent> who(new rndenv(args)); // TODO: change to your environment agent
			lounge[who->name()] = who;
		} else if (para.find("--save") == 0) {
			save_statistics = true;
		} else if (para.find("--debug") == 0) {
			debug_mode = true;
		}
	}

	std::regex match_open  ("^match \\S+ open \\S+:\\S+$");
	std::regex match_take  ("^match \\S+ (play|evil) take turn$");
	std::regex match_move  ("^match \\S+ (play|evil) move \\S+$");
	std::regex match_close ("^match \\S+ close \\S+$");

	std::ostream& dout = debug_mode ? std::cerr : *(new std::ofstream);
	for (std::string command; std::getline(std::cin, command); ) {
		dout << "<< " << command << std::endl;
		try {
			if (std::regex_match(command, match_take)) {
				std::string id, role, buf;
				std::stringstream(command) >> buf >> id >> role;

				if (arena.find(id) == arena.end()) continue;
				match& m = *arena.at(id);
				agent& who = *(role == "play" ? m.play : m.evil);
				if (who.role() == "dummy") continue;
				action a = who.take_action(m.b);

				std::stringstream oss;
				oss << "match " << id << " " << role << " move " << std::hex << int(a);
				std::string out = oss.str();
				std::cout << out << std::endl;
				dout << ">> " << out << std::endl;

			} else if (std::regex_match(command, match_move)) {
				std::string id, role, buf; int code;
				std::stringstream(command) >> buf >> id >> role >> buf >> std::hex >> code;

				if (arena.find(id) == arena.end()) continue;
				match& m = *arena.at(id);
				statistic& stat = *m.stat;
				int r = action(code).apply(m.b);
				dout << m.b << "reward = " << r << std::endl;
				if (r != -1) {
					stat.save_action(code);
				}

			} else if (std::regex_match(command, match_open)) {
				std::string id, info, buf;
				std::stringstream(command) >> buf >> id >> buf >> info;

				if (arena.find(id) != arena.end()) {
					dout << id << " is running now" << std::endl;
					continue;
				}

				arena[id] = std::shared_ptr<match>(new match(id, info));
				match& m = *arena.at(id);
				statistic& stat = *m.stat;
				stat.open_episode(info);
				m.b = stat.make_empty_board();

				auto open_episode_for = [&](std::string name) -> std::shared_ptr<agent> {
					auto it = std::find_if(lounge.begin(), lounge.end(),
							[=](std::pair<std::string, std::shared_ptr<agent>> p) {
						return p.second->name() == name;
					});
					if (it == lounge.end()) return {};
					std::string role = it->second->role().find("play") == 0 ? "play" : "evil";
					std::shared_ptr<agent>& who = role == "play" ? m.play : m.evil;
					who = it->second;
					who->open_episode(role == "play" ? "~:" + m.evil->name() : m.play->name() + ":~");

					std::stringstream oss;
					oss << "match " << id << " " << role << " ready" << std::endl;
					std::string out = oss.str();
					std::cout << out << std::endl;
					dout << ">> " << out << std::endl;
					return who;
				};

				bool as_play = open_episode_for(m.play->name()) != nullptr;
				bool as_evil = open_episode_for(m.evil->name()) != nullptr;
				if (!(as_play || as_evil)) {
					dout << info << " not found" << std::endl;
					arena.erase(id);
				}

			} else if (std::regex_match(command, match_close)) {
				std::string id, win, buf;
				std::stringstream(command) >> buf >> id >> buf >> win;

				if (arena.find(id) == arena.end()) continue;
				match& m = *arena.at(id);
				statistic& stat = *m.stat;
				stat.close_episode(win);
				m.play->close_episode(win);
				m.evil->close_episode(win);

				if (save_statistics) {
					std::string save = id + "-" + win + ".stat";
					dout << "saving statistic to " << save << "..." << std::endl;
					std::ofstream out;
					out.open(save.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
					if (out.is_open()) {
						out << stat;
						out.close();
					}
				}

				arena.erase(id);
			}
		} catch (std::logic_error&) {
		}

	}

	return 0;
}


/**
* 这里只记录了最近1000局(limit)的盘面信息以及统计结果，没有保存weight
*/
void save_statistic(const statistic stat, const std::string save){
	// std::cout << "begin save_statistic" << std::endl;
	if (save.size()) {
		std::ofstream out;
		out.open(save.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) return;
		out << stat;
		out.flush();
		out.close();
	}
	// std::cout << "end save_statistic" << std::endl;
}

int main(int argc, const char* argv[]) {
	clock_t startTime,endTime;
	startTime = clock();
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
		} else if (para.find("--shell") == 0) {
			return shell(argc, argv);
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
		// std::cout << game << std::endl;
		// action::place(3, 0).apply(game);
        while (true) {
			// std::cout << game << std::endl;
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
		// if(i % 200000 == 0)
		// 	play.change_learning_rate();

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
	// std::cout << "aaaa: " << aaaa << std::endl;
	return 0;
}
