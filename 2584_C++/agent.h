#pragma once
#include <string>
#include <random>
#include <sstream>
#include <map>
#include <type_traits>
#include <algorithm>
#include "board.h"
#include "action.h"
#include "weight.h"

class agent {
public:
	agent(const std::string& args = "") {
		std::stringstream ss(args);
		for (std::string pair; ss >> pair; ) {
			std::string key = pair.substr(0, pair.find('='));
			std::string value = pair.substr(pair.find('=') + 1);
			property[key] = { value };
		}
	}
	virtual ~agent() {}
	virtual void open_episode(const std::string& flag = "") {}
	virtual void close_episode(const std::string& flag = "") {}
	virtual action take_action(const board& b) { return action(); }
	virtual bool check_for_win(const board& b) { return false; }

public:
	virtual std::string name() const {
		auto it = property.find("name");
		return it != property.end() ? std::string(it->second) : "unknown";
	}
protected:
	typedef std::string key;
	struct value {
		std::string value;
		operator std::string() const { return value; }
		template<typename numeric, typename = typename std::enable_if<std::is_arithmetic<numeric>::value, numeric>::type>
		operator numeric() const { return numeric(std::stod(value)); }
	};
	std::map<key, value> property;
};

/**
 * evil (environment agent)
 * add a new random tile on board, or do nothing if the board is full
 * 1-tile: 90%
 * 2-tile: 10%
 */
class rndenv : public agent {
public:
	rndenv(const std::string& args = "") : agent("name=rndenv " + args) {
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
	}

	virtual action take_action(const board& after) {
		int space[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15 };
		std::shuffle(space, space + 16, engine);
		// std::cout << after << std::endl;
		for (int pos : space) {
			if (after(pos) != 0) continue;
			std::uniform_int_distribution<int> popup(0, 9);
			int tile = popup(engine) ? 1 : 2;
			return action::place(tile, pos);
		}
		return action();
	}

private:
	std::default_random_engine engine;
};

/**
 * player (dummy)
 * select an action randomly
 */
class player : public agent {
public:
	player(const std::string& args = "") : agent("name=player " + args),  alpha(0.0025f) {
		if (property.find("seed") != property.end())
			engine.seed(int(property["seed"]));
		if (property.find("alpha") != property.end())
			alpha = float(property["alpha"]);

		if (property.find("load") != property.end())
			load_weights(property["load"]);
		// TODO: initialize the n-tuple network
		// const int feature_num = 16 * 16 * 16 * 16;
		// float n_tuple[feature_num];
		
	}

	~player() {
		if (property.find("save") != property.end())
			save_weights(property["save"]);
	}

	virtual void open_episode(const std::string& flag = "") {
		episode.clear();
		episode.reserve(32768);
	}

	virtual void close_episode(const std::string& flag = "") {
		// TODO: train the n-tuple network by TD(0)
	}

	virtual action take_action(const board& before) {
		action best;
		// TODO: select a proper action
		// best = choose_action(before);
		// TODO: push the step into episode
		return best;
	}

public:
	virtual void load_weights(const std::string& path) {
		std::ifstream in;
		in.open(path.c_str(), std::ios::in | std::ios::binary);
		if (!in.is_open()) std::exit(-1);
		size_t size;
		in.read(reinterpret_cast<char*>(&size), sizeof(size));
		weights.resize(size);
		for (weight& w : weights)
			in >> w;
		in.close();
	}

	virtual void save_weights(const std::string& path) {
		std::ofstream out;
		out.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		size_t size = weights.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : weights)
			out << w;
		out.flush();
		out.close();
	}

private:
	std::vector<weight> weights;

	struct state {
		// TODO: select the necessary components of a state
		board before;
		board after;
		action move;
		int reward;
	};

	std::vector<state> episode;
	float alpha;

private:
	std::default_random_engine engine;
};
