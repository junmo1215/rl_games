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
 
 int main(int argc, const char* argv[]) {
     std::cout << "2048-Demo: ";
     std::copy(argv, argv + argc, std::ostream_iterator<const char*>(std::cout, " "));
     std::cout << std::endl << std::endl;
 
     size_t total = 1000, block = 0;
     std::string play_args, evil_args;
     std::string load, save;
     bool summary = false;
     for (int i = 1; i < argc; i++) {
         std::string para(argv[i]);
         if (para.find("--total=") == 0) {
             total = std::stoull(para.substr(para.find("=") + 1));
         } else if (para.find("--block=") == 0) {
             block = std::stoull(para.substr(para.find("=") + 1));
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
 
     statistic stat(total, block);
 
     if (load.size()) {
         std::ifstream in;
         in.open(load.c_str(), std::ios::in | std::ios::binary);
         if (!in.is_open()) return -1;
         in >> stat;
         in.close();
     }
 
     player play(play_args);
     rndenv evil(evil_args);
 
     while (!stat.is_finished()) {
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
 