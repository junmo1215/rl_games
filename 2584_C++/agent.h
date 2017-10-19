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
		board board_terminal = episode[episode.size() - 1].after;

		train_weights(board_terminal);
		// std::vector<long long> features = get_features(b_terminal);
		
		// for(int j = 0; j < 4; j++){
		// 	// std::cout << "before learning " << weights[i][features[i]] << std::endl;
		// 	weights[j][features[j]] += alpha * (0.0 - lookup_value(features));
		// 	// std::cout << "after learning " << weights[i][features[i]] << std::endl;
		// }

		// std::cout << "updating !!" << std::endl;
		// before = episode[episode.size() - 2].after;
		for(int i = episode.size() - 2; i >= 0; i--){
			state step = episode[i];
			state step_next = episode[i + 1];

			// std::cout << "before:" << std::endl;
			// std::cout << step.before;
			// std::cout << "actions:" << step.move.name() << std::endl;
			// std::cout << "reward:" << step.reward << std::endl;
			// std::cout << "after:" << std::endl;
			// std::cout << step.after;
			train_weights(step.after, step_next.after, step_next.reward);

			// std::vector<long long> features = get_features(step.after);
			
			// for(int j = 0; j < 4; j++){
			// 	// std::cout << "before learning " << weights[i][features[i]] << std::endl;
			// 	weights[j][features[j]] += alpha * (step_t_1.reward + board_value(step_t_1.after) - lookup_value(features));
			// 	// std::cout << "after learning " << weights[i][features[i]] << std::endl;
			// }	
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
	 * 
	 * flag: 判断旋转和对称的标志
	 * 	0：向右旋转0次（原始索引，不翻转不对称）
	 *  1：向右旋转0次，水平翻折
	 *  2：向右旋转1次
	 *  3：向右旋转1次，水平翻折
	 *  4：向右旋转2次
	 *  5：向右旋转2次，水平翻折
	 *  6：向右旋转3次
	 *  7：向右旋转3次，水平翻折
	 */
	std::vector<long long> get_features(const board& before, const int flag){
		std::vector<long long> features;

		std::vector<std::array<int, 4>> indexs;
		indexs.push_back({0, 4, 8, 12});
		indexs.push_back({1, 5, 9, 13});
		indexs.push_back({0, 1, 2, 3});
		indexs.push_back({4, 5, 6, 7});

		// if(print)
		// 	print_index(indexs);

		// std::cout << "flag: " << flag << std::endl;
		// int indexs[4][4] = {
		// 	{0, 4, 8, 12},
		// 	{1, 5, 9, 13},
		// 	{0, 1, 2, 3},
		// 	{4, 5, 6, 7}
		// };
		// int index1[4] = {0, 4, 8, 12};
		// int index2[4] = {1, 5, 9, 13};
		// int index3[4] = {0, 1, 2, 3};
		// int index4[4] = {4, 5, 6, 7};
		for(int i = 0; i < flag / 2; i++){
			// std::cout << "rotate_right" << std::endl;
			indexs = rotate_right(indexs);
		}

		if(flag % 2 == 1){
			// std::cout << "reflect_horizontal" << std::endl;
			indexs = reflect_horizontal(indexs);
		}

		// if(print)
		// 	print_index(indexs);
		// std::cout << std::endl;
		
		features.push_back(get_feature(before, indexs[0]));
		features.push_back(get_feature(before, indexs[1]));
		features.push_back(get_feature(before, indexs[2]));
		features.push_back(get_feature(before, indexs[3]));
		return features;
	}
	
	void print_index(const std::vector<std::array<int, 4>> indexs){
		for(int i = 0; i < 4; i++){
			for(int j = 0; j < 4; j++){
				std::cout << indexs[i][j] << "\t";
			}
			std::cout << std::endl;
		}
	}

	long long get_feature(const board& b, const std::array<int, 4> indexs){
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
		return lookup_value(get_features(b, 0));
	}

	void train_weights(const board& b, const board& next_b, const int reward){
		// 旋转对称加起来导致局面有八种变化
		for(int i = 0; i < 8; i++){
			std::vector<long long> features = get_features(b, i);
			std::vector<long long> next_features = get_features(next_b, i);
			
			for(int j = 0; j < 4; j++){
				// std::cout << "before learning " << weights[i][features[i]] << std::endl;
				weights[j][features[j]] += alpha * (reward + lookup_value(next_features) - lookup_value(features));
				// std::cout << "after learning " << weights[i][features[i]] << std::endl;
			}
		}
			
	}

	void train_weights(const board& b){
		// std::cout << "enter train_weights" << std::endl;

		// 旋转对称加起来导致局面有八种变化
		for(int i = 0; i < 8; i++){
			std::vector<long long> features = get_features(b, i);

			for(int j = 0; j < 4; j++){
				// std::cout << "before learning " << weights[i][features[i]] << std::endl;
				weights[j][features[j]] += alpha * (0.0 - lookup_value(features));
				// std::cout << "after learning " << weights[i][features[i]] << std::endl;
			}
		}
	}

	std::vector<std::array<int, 4>> rotate_right(std::vector<std::array<int, 4>> indexs){
		for (int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++){
				int temp = indexs[i][j];
				indexs[i][j] = (temp % 4) * 4 - (temp / 4) + 3;
			}
		}
		return indexs;
	}

	std::vector<std::array<int, 4>> reflect_horizontal(std::vector<std::array<int, 4>> indexs){
		// std::vector<std::array<int, 4>> result;
		for (int i = 0; i < 4; i++) {
			for(int j = 0; j < 4; j++){
				int temp = indexs[i][j];
				indexs[i][j] = (temp / 4) * 4 - (temp % 4) + 3;
			}
		}
		return indexs;
	}
};
