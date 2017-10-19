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
	virtual action take_action(const board& b, const bool print=false) { return action(); }
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

	virtual action take_action(const board& after, const bool print=false) {
		// std::cout << "enter rndenv take_action" << std::endl;
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
		// initialize the n-tuple network
		const long long feature_num = POSSIBLE_INDEX * POSSIBLE_INDEX * POSSIBLE_INDEX * POSSIBLE_INDEX;
		weights.push_back(weight(feature_num));
		weights.push_back(weight(feature_num));
		weights.push_back(weight(feature_num));
		weights.push_back(weight(feature_num));
	}

	~player() {
		if (property.find("save") != property.end())
			save_weights(property["save"]);
	}

	virtual void open_episode(const std::string& flag = "") {
		episode.clear();
		episode.reserve(327680);
	}

	virtual void close_episode(const std::string& flag = "") {
		// train the n-tuple network by TD(0)
		// for(state step : episode){
		board before = episode[episode.size() - 1].after;
		std::vector<long long> features = get_features(before);
		
		for(int j = 0; j < 4; j++){
			// std::cout << "before learning " << weights[i][features[i]] << std::endl;
			weights[j][features[j]] += alpha * (0.0 - board_value(before));
			// std::cout << "after learning " << weights[i][features[i]] << std::endl;
		}
		// std::cout << "updating !!" << std::endl;
		// before = episode[episode.size() - 2].after;
		for(int i = episode.size() - 2; i >= 0; i--){
			before = episode[i + 1].after;
			state step = episode[i];

			// std::cout << "before:" << std::endl;
			// std::cout << step.before;
			// std::cout << "actions:" << step.move.name() << std::endl;
			// std::cout << "reward:" << step.reward << std::endl;
			// std::cout << "after:" << std::endl;
			// std::cout << step.after;


			std::vector<long long> features = get_features(step.after);
			
			for(int j = 0; j < 4; j++){
				// std::cout << "before learning " << weights[i][features[i]] << std::endl;
				weights[j][features[j]] += alpha * (episode[i + 1].reward + board_value(before) - board_value(step.after));
				// std::cout << "after learning " << weights[i][features[i]] << std::endl;
			}	
			// std::cin.ignore();
			// std::cout << "\033[2J\033[1;1H";
		}
		// std::cout << "updating end -- 1!!" << std::endl;
	}

	virtual action take_action(const board& before, const bool print=false) {
		if(print)
			std::cout << "enter player take_action" << std::endl;

		// select a proper action
		int best_op = 0;
		float best_vs = -99999999;
		board after;
		int best_reward = 0;

		// 模拟四种动作，取实验后盘面最好的动作作为best
		for(int op: {0, 1, 2, 3}){
			board b = before;
			int reward = b.move(op);
			if(print)
				printf("op: %d %s reward: %d \n", op, action(op).name().c_str(), reward);
			if(reward != -1){
				float v_s = board_value(b);
				if(v_s > best_vs){
					best_op = op;
					best_vs = v_s;
					after = b;
					best_reward = reward;
				}
			}
		}
		action best(best_op);
		if(print)
			std::cout << best.name() << std::endl;
		// int opcode[] = {0, 1, 2, 3};
		// std::shuffle(opcode, opcode + 4, engine);
		// for(int op: opcode){
		// 	board b = before;
		// 	if(b.move(op) != -1){
		// 		return action::move(op);
		// 	}
		// }
		// action best(best_op);

		// best = choose_action(before);
		// std::vector<int> features = get_features(before);
		// std::cout << before;
		// std::cout << features[0] << std::endl;
		// std::cout << features[1] << std::endl;
		// std::cout << features[2] << std::endl;
		// std::cout << features[3] << std::endl;

		// TODO: push the step into episode
		struct state step = {before, after, best, best_reward};
		// std::cout << step.before << std::endl;
		// std::cout << step.reward << std::endl;
		episode.push_back(step);
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

private:
	/**
	 * 根据当前局面提取特征
	 */
	std::vector<long long> get_features(const board& before){
		std::vector<long long> features;
		int index1[4] = {0, 4, 8, 12};
		int index2[4] = {1, 5, 9, 13};
		int index3[4] = {2, 6, 10, 14};
		int index4[4] = {3, 7, 11, 15};
		features.push_back(get_feature(before, index1));
		features.push_back(get_feature(before, index2));
		features.push_back(get_feature(before, index3));
		features.push_back(get_feature(before, index4));
		return features;
	}

	long long get_feature(const board& b, const int indexs[4]){
		long long result = 0;
		for(int i = 0; i < 4; i++){
			result *= 32;
			int r = indexs[i] / 4;
			int c = indexs[i] % 4;
			result += b[r][c];
		}
		return result;
	}

	/**
	 * 根据局面解析出来的feature获取当前的状态动作值
	 */
	float lookup_value(const std::vector<long long> features){
		float v_s = 0;
		for(int i = 0; i < 4; i++){
			v_s += weights[i][features[i]];
		}
		return v_s;
	}

	/**
	 * 获取盘面的估值
	 */
	float board_value(const board& b){
		return lookup_value(get_features(b));
	}
};
