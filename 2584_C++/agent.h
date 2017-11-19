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
			std::cout << key << "\t" << value << std::endl;
		}
		std::cout << std::endl;
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

		if (property.find("load") != property.end()){
			load_weights(property["load"]);
		}
		else{
			// initialize the n-tuple network
			const long feature_num = MAX_INDEX * MAX_INDEX * MAX_INDEX * MAX_INDEX * MAX_INDEX * MAX_INDEX;
			for(int i = 0; i < TUPLE_NUM; i++){
				weights.push_back(weight(feature_num));
			}
		}
	}

	~player() {
		save_weights();
	}

	virtual void open_episode(const std::string& flag = "") {
		episode.clear();
		episode.reserve(327680);
	}

	virtual void close_episode(const std::string& flag = "") {
		// train the n-tuple network by TD(0)
		train_weights(episode[episode.size() - 1].after);
		for(int i = episode.size() - 2; i >= 0; i--){
			state step_next = episode[i + 1];

			train_weights(episode[i].after, step_next.after, step_next.reward);
		}
	}

	virtual action take_action(const board& before) {
		// select a proper action
		int best_op = 0;
		float best_vs = MIN_FLOAT;
		board after;
		int best_reward = 0;

		// 模拟四种动作，取实验后盘面最好的动作作为best
		for(int op: {3, 2, 1, 0}){
			board b = before;
			int reward = b.move(op);
			if(reward != -1){
				float v_s = board_value(b) + reward;
				if(v_s > best_vs){
					best_op = op;
					best_vs = v_s;
					after = b;
					best_reward = reward;
				}
			}
		}
		action best(best_op);

		// push the step into episode
		// 如果这一步有效（改变过best_vs的值）才放入episode里面
		if(best_vs != MIN_FLOAT){
			struct state step = {after, best_reward};
			episode.push_back(step);
		}
		return best;
	}

public:
	virtual void load_weights(const std::string& path) {
		std::cout << "loading weights... " << std::endl;
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
		std::cout << "saving weights to " << path.c_str() << std::endl;
		std::ofstream out;
		out.open(path.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
		if (!out.is_open()) std::exit(-1);
		size_t size = weights.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size));
		for (weight& w : weights){
			out << w;
		}
		out.flush();
		out.close();
		std::cout << "save weights success " << std::endl;
	}

	virtual void save_weights(){
		if (property.find("save") != property.end())
			save_weights(property["save"]);
	}

	virtual void change_learning_rate(){
		std::cout << "learning rate: " << alpha;
		alpha /= 2;
		std::cout << " change to : " << alpha << std::endl;
	}

	const static std::array<std::array<int, TUPLE_LENGTH>, TUPLE_NUM> indexs;

private:
	std::vector<weight> weights;

	struct state {
		// select the necessary components of a state
		board after;
		int reward;
	};

	std::vector<state> episode;
	float alpha;

private:
	std::default_random_engine engine;

private:
	/**
	 * 根据局面和位置获取对应的feature
	 * 
	 * 由于最大index之前写的是24会造成weight里面存不下去
	 * 并且出现的概率很小，所以添加合并index的步骤
	 * 在提取feature的时候不区分20,21,22,23,24等（这里假设21是MAX_INDEX）
	 * 这样做虽然最后达到的最大分数会稍微低一点，并且胜率也不会太高
	 * 但是收敛速度会稍微快些，并且节省了很多内存
	 */
	long get_feature(const board& b, const std::array<int, TUPLE_LENGTH> index){
		long result = 0;
		int tile;
		for(int i : index){
			result *= MAX_INDEX;
			// 通过获取地址的方式拿到索引比之前的做法要稍微快一点
			// 下面几种写法结果相同，只有速度上有些差异
			// tile = b[i / 4][i % 4];
			// tile = b[i >> 2][i & 0b11];
			tile = *(&(b[0][0]) + i);
			// tile = b.at(i);
			// 目前我電腦12G內存沒辦法跑之前的代碼
			if(tile >= (MAX_INDEX - 1)){
				result += (MAX_INDEX - 1);
			}
			else{
				result += tile;
			}
		}
		return result;
	}

	/**
	 * 获取盘面的估值
	 */
	float board_value(const board& b){
		float result = 0;
		for(int i = 0; i < TUPLE_NUM; i++){
			result += weights[i][get_feature(b, indexs[i])];
		}
		return result;
	}

	void train_weights(const board& b, const board& next_b, const int reward){
		// 这个写法比之前的速度快，并且逻辑上更加说得通一点
		// 在更新weight的同时不应该由于前面几次循环中调整了weight而修改board value
		float delta = alpha * (reward + board_value(next_b) - board_value(b));
		for(int i = 0; i < TUPLE_NUM; i++){
			weights[i][get_feature(b, indexs[i])] += delta;
		}
	}

	void train_weights(const board& b){
		float delta = - alpha * board_value(b);
		for(int i = 0; i < TUPLE_NUM; i++){
			weights[i][get_feature(b, indexs[i])] += delta;
		}
	}
};

// N-tuple中feature的索引，其中前面四行是基础的索引（两个斧头形状两个2 * 3的box）
// 前面四组是由第一组旋转得到的新的索引，后面四组是前面四组对称得到的
const std::array<std::array<int, TUPLE_LENGTH>, TUPLE_NUM> player::indexs = {{
	{{0, 4, 8, 9, 12, 13}},
	{{1, 5, 9, 10, 13, 14}},
	{{1, 2, 5, 6, 9, 10}},
	{{2, 3, 6, 7, 10, 11}},
	
	{{3, 2, 1, 5, 0, 4}},
	{{7, 6, 5, 9, 4, 8}},
	{{7, 11, 6, 10, 5, 9}},
	{{11, 15, 10, 14, 9, 13}},

	{{15, 11, 7, 6, 3, 2}},
	{{14, 10, 6, 5, 2, 1}},
	{{14, 13, 10, 9, 6, 5}},
	{{13, 12, 9, 8, 5, 4}},

	{{12, 13, 14, 10, 15, 11}},
	{{8, 9, 10, 6, 11, 7}},
	{{8, 4, 9, 5, 10, 6}},
	{{4, 0, 5, 1, 6, 2}},


	{{3, 7, 11, 10, 15, 14}},
	{{2, 6, 10, 9, 14, 13}},
	{{2, 1, 6, 5, 10, 9}},
	{{1, 0, 5, 4, 9, 8}},

	{{0, 1, 2, 6, 3, 7}},
	{{4, 5, 6, 10, 7, 11}},
	{{4, 8, 5, 9, 6, 10}},
	{{8, 12, 9, 13, 10, 14}},

	{{12, 8, 4, 5, 0, 1}},
	{{13, 9, 5, 6, 1, 2}},
	{{13, 14, 9, 10, 5, 6}},
	{{14, 15, 10, 11, 6, 7}},

	{{15, 14, 13, 9, 12, 8}},
	{{11, 10, 9, 5, 8, 4}},
	{{11, 7, 10, 6, 9, 5}},
	{{7, 3, 6, 2, 5, 1}}
}};
