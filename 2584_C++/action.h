#pragma once
#include <string>
#include "board.h"

class action {
public:
	action(const action& act) : opcode(act) {}
	action(const int& op = -1) : opcode(op) {}
	operator int() const { return opcode; }

	int apply(board& b) const {
		if ((0b11 & opcode) == (opcode)) {
			// 玩家移动
			// player action (slide up, right, down, left)
			return b.move(opcode);
		} else if (b(opcode & 0x0f) == 0) {
			// 环境放置新的tile
			// environment action (place a new tile)
			b(opcode & 0x0f) = (opcode >> 4);
			// std::cout << opcode << std::endl;
			// std::cout << b << std::endl;
			// throw;
			return 0;
		}
		return -1;
	}

	std::string name() const {
		if ((0b11 & opcode) == (opcode)) {
			std::string opname[] = { "up", "right", "down", "left" };
			return "slide " + opname[opcode];
		} else {
			return "place " + std::to_string(opcode >> 4) + "-index at position " + std::to_string(opcode & 0x0f);
		}
		return "null";
	}

	static action move(const int& oper) {
		return action(oper);
	}

	// 这里把序号和位置放在了一起，最后四位表示放置的位置(0000~1111)，前面是数列的索引
	static action place(const int& tile, const int& pos) {
		return action((tile << 4) | (pos));
	}

private:
	const int opcode;
};
