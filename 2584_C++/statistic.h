#pragma once
#include <list>
#include <vector>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <numeric>
#include "board.h"
#include "action.h"
#include "agent.h"

class statistic {
public:
	/**
	 * the total episodes to run
	 * the block size of statistic
	 * the limit of saving records
	 *
	 * note that total >= limit >= block
	 */
	statistic(const size_t& total, const size_t& block = 0, const size_t& limit = 0)
		: total(total),
		  block(block ? block : this->total),
		  limit(std::max(limit, this->block)),
		  count(0) {}

public:
	/**
	* show the statistic of last 'block' games
	*
	* the format would be
	* 10000	avg = 46172, max = 137196, ops = 218817
	*	233 	100%	(0.1%)
	*	377 	99.9%	(0.2%)
	*  	610 	99.7%	(1.5%)
	*	987 	98.2%	(4.7%)
	*	1597	93.5%	(32%)
	*	2584	61.5%	(41.2%)
	*	4181	20.3%	(17.7%)
	*	6765	2.6%	(2.6%)
	*
	* where (assume that block = 1000)
	*  '10000': current index (n)
	*  'avg = 46172': the average score of saved games is 46172
	*  'max = 137196': the maximum score of saved games is 137196
	*  'ops = 218817': the average speed of saved games is 218817
	*  '61.5%': 61.5% (615 games) reached 2584-tiles in saved games (a.k.a. win rate of 2584-tile)
	*  '41.2%': 41.2% (412 games) terminated with 2584-tiles (the largest) in saved games
	*/
	void show() const {
		int block = std::min(data.size(), this->block);
		size_t sum = 0, max = 0, opc = 0, stat[POSSIBLE_INDEX] = { 0 };
		uint64_t duration = 0;
		auto it = data.end();
		for (int i = 0; i < block; i++) {
			auto& path = *(--it);
			board game;
			size_t score = 0;
			for (const action& move : path)
				score += move.apply(game);
			sum += score;
			max = std::max(score, max);
			opc += (path.size() - 2) / 2;
			int tile = 0;
			for (int i = 0; i < 16; i++)
				tile = std::max(tile, game(i));
			stat[tile]++;
			duration += (path.tock_time() - path.tick_time());
		}
		float avg = float(sum) / block;
		float coef = 100.0 / block;
		float ops = opc * 1000.0 / duration;
		std::cout << count << "\t";
		std::cout << "avg = " << unsigned(avg) << ", ";
		std::cout << "max = " << unsigned(max) << ", ";
		std::cout << "ops = " << unsigned(ops) << std::endl;
		// t表示的是数列的序号
		// stat[i]: 数列中第i个元素是这次游戏中最大值的次数
		// c表示的是在这个循环里面一共统计了多少次最大次数，按照block=1000的情况就是c不会大于1000
		for (int t = 0, c = 0; c < block; c += stat[t++]) {
			if (stat[t] == 0) continue;
			int accu = std::accumulate(stat + t, stat + POSSIBLE_INDEX, 0);
			std::cout << "\t" << fibonacci[t] << "\t" << (accu * coef) << "%";
			std::cout << "\t(" << (stat[t] * coef) << "%)" << std::endl;
		}
		std::cout << std::endl;
	}

	void summary() const {
		auto block_temp = block;
		const_cast<statistic&>(*this).block = data.size();
		show();
		const_cast<statistic&>(*this).block = block_temp;
	}

	bool is_finished() const {
		return count >= total;
	}

	void open_episode(const std::string& flag = "") {
		if (count++ >= limit) data.pop_front();
		data.emplace_back();
		data.back().tick();
	}

	void close_episode(const std::string& flag = "") {
		data.back().tock();
		if (count % block == 0) show();
	}

	board make_empty_board() {
		return board();
	}

	void save_action(const action& move) {
		data.back().push_back(move);
	}

	agent& take_turns(agent& play, agent& evil) {
		return (std::max(data.back().size() + 1, size_t(2)) % 2) ? play : evil;
	}

	agent& last_turns(agent& play, agent& evil) {
		return take_turns(evil, play);
	}

	// 重写运算符
	friend std::ostream& operator <<(std::ostream& out, const statistic& stat) {
		auto size = stat.data.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (const record& rec : stat.data) out << rec;
		return out;
	}

	friend std::istream& operator >>(std::istream& in, statistic& stat) {
		auto size = stat.data.size();
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		stat.count = size;
		stat.data.resize(size);
		for (record& rec : stat.data) in >> rec;
		return in;
	}

private:
	class record : public std::vector<action> {
	public:
		record() { reserve(32768); }
		void tick() { time[0] = milli(); }
		void tock() { time[1] = milli(); }
		uint64_t tick_time() const { return time[0]; }
		uint64_t tock_time() const { return time[1]; }

		friend std::ostream& operator <<(std::ostream& out, const record& rec) {
			auto size = rec.size();
			auto time = rec.time;
			out.write(reinterpret_cast<char*>(&size), sizeof(size));
			for (const action& act : rec) {
				short opcode = int(act);
				out.write(reinterpret_cast<const char*>(&opcode), sizeof(opcode));
			}
			out.write(reinterpret_cast<const char*>(time), sizeof(time[0]) * 2);
			return out;
		}
		friend std::istream& operator >>(std::istream& in, record& rec) {
			auto size = rec.size();
			auto time = rec.time;
			in.read(reinterpret_cast<char*>(&size), sizeof(size));
			rec.reserve(size);
			for (size_t i = 0; i < size; i++) {
				short opcode;
				in.read(reinterpret_cast<char*>(&opcode), sizeof(opcode));
				rec.emplace_back(int(opcode));
			}
			in.read(reinterpret_cast<char*>(time), sizeof(time[0]) * 2);
			return in;
		}

	private:
		uint64_t milli() const {
			auto now = std::chrono::system_clock::now().time_since_epoch();
			return std::chrono::duration_cast<std::chrono::milliseconds>(now).count();
		}
		uint64_t time[2];
	};

	size_t total;
	size_t block;
	size_t limit;
	size_t count;
	std::list<record> data;
};