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
			// player action (slide up, right, down, left)
			return b.move(opcode);
		} else if (b(opcode & 0x0f) == 0) {
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
	static action place(const int& tile, const int& pos) {
		return action((tile << 4) | (pos));
	}

private:
	const int opcode;
};
