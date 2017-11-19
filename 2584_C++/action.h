#pragma once
#include <string>
#include "board.h"
#include "board2x3.h"

class action {
public:
	action(const action& act) : opcode(act) {}
	action(const int& op = -1) : opcode(op) {}
	operator int() const { return opcode; }

public:
	action& operator =(const action& a) { opcode = a; return *this; }
	bool operator ==(const action& a) const { return opcode == int(a); }
	bool operator < (const action& a) const { return opcode <  int(a); }
	bool operator !=(const action& a) const { return !(*this == a); }
	bool operator > (const action& a) const { return a < *this; }
	bool operator <=(const action& a) const { return !(a < *this); }
	bool operator >=(const action& a) const { return !(*this < a); }

public:

	int apply(board2x3& b) const {
		if ((0b11 & opcode) == (opcode)) {
			// player action (slide up, right, down, left)
			return b.move(opcode);
		} else if (b(opcode & 0x0f) == 0) {
			// environment action (place a new tile)
			b(opcode & 0x0f) = (opcode >> 4);
			return 0;
		}
		return -1;
	}

	int apply(board& b) const {
		if ((0b11 & opcode) == (opcode)) {
			// 玩家移动
			// player action (slide up, right, down, left)
			return b.move(opcode);
		} else if (b(opcode & 0x0f) == 0) {
			// 环境放置新的tile
			// environment action (place a new tile)
			b(opcode & 0x0f) = (opcode >> 4);
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
	int opcode;
};
