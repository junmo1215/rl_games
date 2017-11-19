
/**
 * 用一个2进制数来表示一个盘面而不是使用之前的那种二维array的方式
 * 参考这个文件中的board类
 *  https://github.com/moporgic/TDL2048-Demo/blob/master/2048.cpp
 * 用一个uint64_t数字(raw)代表盘面
 * 
 * 不过由于2584盘面的可能情况远远大于2048的情况，所以这里采用两个数的方式代替
 * 2048可能情况： 16^16 = (2^4)^14 = 2^64 
 *  (刚好可以用一个uint64_t表示，这里有部分简化，理想情况应该是17^16，但是这个情况几乎不会出现)
 * 2584可能情况： 32^16 = 2^80
 *  (一个数字存不下，考虑分两个数字存储，一个uint16_t(2^16)，一个uint64_t(2^64))
 */ 
#pragma once

class bitboard {

public:
    bitboard() : _left(0), _right(0){}
    bitboard(const uint64_t& right) : _left(0), _right(right){}
    bitboard(const uint16_t& left, const uint64_t& right) : _left(left), _right(right) {}

	// bitboard(const bitboard& b) = default;
    // bitboard& operator =(const bitboard& b) = default;
    
    // 重载运算符 &
    bitboard operator &(const bitboard& b) const{
        return bitboard(_left & b._left, _right & b._right);
    }

    // 重载运算符 &=
    bitboard& operator &=(const bitboard& b){
        _left &= b._left;
        _right &= b._right;
        return *this;
    }

    // 重载运算符 |
    bitboard operator |(const bitboard& b) const{
        return bitboard(_left | b._left, _right | b._right);
    }

    // 重载运算符 |=
    bitboard& operator |=(const bitboard& b){
        _left |= b._left;
        _right |= b._right;
        return *this;
    }

    // 重载运算符 ~
    bitboard operator ~() const{
        return bitboard(~_left, ~_right);
    }

    // 重载位运算
    bitboard operator <<(const int& shift_num){
        if(shift_num == 0)
            return *this;
        bitboard result = *this;
        if(shift_num < 16){
            result._left <<= shift_num;
            result._left |= _right >> (64 - shift_num);
            result._right <<= shift_num;
        }
        else if(shift_num < 64) {
            result._left = _right >> (64 - shift_num);
            result._right <<= shift_num;
        }
        else {
            result._left = _right << (shift_num - 64);
            result._right = 0;
        }
        return result;
    }

    bitboard& operator<<=(const int& shift_num)
    {
        if(shift_num == 0)
            return *this;
        if(shift_num < 16){
            _left <<= shift_num;
            _left |= _right >> (64 - shift_num);
            _right <<= shift_num;
        }
        else if(shift_num < 64) {
            _left = _right >> (64 - shift_num);
            _right <<= shift_num;
        }
        else {
            _left = _right << (shift_num - 64);
            _right = 0;
        }
        return *this;
    }

    bitboard operator>>(const int& shift_num)
    {
        if(shift_num == 0)
            return *this;
        bitboard result = *this;
        // 小于16和小于64的情况似乎要分开写，不知道是不是编译器参数设置的问题
        // 目前设置的情况下，下面结果可以看出问题
        // uint16_t a = 10;
        // for(int i = 0; i < 64; i++){
        //     cout << i << "\t" << (a >> i) << endl;
        // }
        if(shift_num < 16){
            result._right = (_right >> shift_num) | (uint64_t)_left << (64 - shift_num); //(((uint64_t)(_left << (16 - shift_num))) << (64 - shift_num));
            result._left >>= shift_num;
        }
        else if(shift_num < 64){
            result._right = (_right >> shift_num) | (uint64_t)_left << (64 - shift_num);
            result._left = 0;
        }
        else{
            result._right = _left >> (shift_num - 64);
            result._left = 0;
        }
        // if(shift_num < 64) {
        //     result._right >>= shift_num;
        //     result._right |= (uint64_t)_left << (64 - shift_num);
        //     result._left >>= shift_num;
        // }
        // else {
        //     result._right = _left >> (shift_num - 64);
        //     result._left = 0;
        // }
        return result;
    }

    bitboard& operator>>=(const int& shift_num)
    {
        if(shift_num == 0)
            return *this;
        if(shift_num < 16){
            _right = (_right >> shift_num) | (uint64_t)_left << (64 - shift_num);
            _left >>= shift_num;
        }
        else if(shift_num < 64){
            _right = (_right >> shift_num) | (uint64_t)_left << (64 - shift_num);
            _left = 0;
        }
        else{
            _right = _left >> (shift_num - 64);
            _left = 0;
        }
        // _right >>= shift_num;
        // if(shift_num < 64) {
        //     _right |= (uint64_t)_left << (64 - shift_num);
        //     _left >>= shift_num;
        // }
        // else {
        //     _right = (uint64_t)_left >> (shift_num - 64);
        //     _left = 0;
        // }
        return *this;
    }

	// operator uint64_t() const { return raw; }

	/**
	 * get a 20-bit row
	 */
	uint32_t fetch(const int& i) const {
        bitboard b = *this;
        return ((b >> (i * 20)) & 0xfffff)._right;
    }

	/**
	 * set a 20-bit row
	 */
	void place(const int& i, const int& r) {
        *this = (*this & ~(bitboard(0xfffff) << (i * 20))) | (bitboard(r & 0xfffff) << (i * 20)); 
    }

	/**
	 * get a 5-bit tile
	 */
	int at(const int& i) const {
        bitboard b = *this;
        // bitboard test = (b >> (i * 5));
        // std::cout << test._left << std::endl;
        // std::cout << test._right << std::endl;
        // return (test & 0x1f)._right;
        return ((b >> (i * 5)) & 0x1f)._right;
    }

	/**
	 * set a 5-bit tile
	 */
	void set(const int& i, const int& t) {
        *this = (*this & ~(bitboard(0x1f) << (i * 5))) | (bitboard(t & 0x1f) << (i * 5));
    }


	// board() : tile() {}
	// board(const board& b) = default;
	// board& operator =(const board& b) = default;

	// std::array<int, 4>& operator [](const int& i) { return tile[i]; }
	// const std::array<int, 4>& operator [](const int& i) const { return tile[i]; }
	// int& operator ()(const int& i) { return tile[i / 4][i % 4]; }
	// const int& operator ()(const int& i) const { return tile[i / 4][i % 4]; }

public:
	bool operator ==(const bitboard& b) const { return _left == b._left && _right == b._right; }
	// bool operator < (const bitboard& b) const { return raw <  b.raw; }
	bool operator !=(const bitboard& b) const { return _left != b._left || _right != b._right; }

	// bool operator > (const bitboard& b) const { return b < *this; }
	// bool operator <=(const bitboard& b) const { return !(b < *this); }
	// bool operator >=(const bitboard& b) const { return !(*this < b); }

private:
    /**
	 * the lookup table for moving board
	 */
	struct lookup {
		uint32_t raw; // base row (20-bit raw)
		uint32_t left; // left operation
		uint32_t right; // right operation
		// 2048的左划和右划得分是一样的，但是2584不同，所以需要分开写
        int score_l; // merge reward
        int score_r;

		void init(const uint32_t& r) {
            /**
             * 这里的r代表行的数字 范围是从 0 到 (2^5)^4 - 1 = 1048575
             * e.g. 
             *  r = 1   2584中的盘面：1 0 0 0
             *  r = 2   2584: 2 0 0 0
             */
            if(r > 0xfffff)
                return;

			raw = r;

            // 分别代表这一行中的tile序号
			uint32_t V[4] = { (r >> 0) & 0x1f, (r >> 5) & 0x1f, (r >> 10) & 0x1f, (r >> 15) & 0x1f };
			uint32_t L[4] = { V[0], V[1], V[2], V[3] };
			uint32_t R[4] = { V[3], V[2], V[1], V[0] }; // mirrored

            // 尝试将这一行左划或者右划合并，这里改变了L和R
            // 所以left和right分别是这一行左划和右划的结果（已经转化为数字）
			score_l = mvleft(L);
			left = ((L[0] << 0) | (L[1] << 5) | (L[2] << 10) | (L[3] << 15));

			score_r = mvleft(R); std::reverse(R, R + 4);
			right = ((R[0] << 0) | (R[1] << 5) | (R[2] << 10) | (R[3] << 15));
		}

        // TODO：没看懂
        // 这里的raw似乎又是代表的整个盘面了
		void move_left(bitboard& b, int& sc, const int& i) const {
			b |= bitboard(left) << (i * 20);
			sc += score_l;
		}

        // TODO：没看懂
		void move_right(bitboard& b, int& sc, const int& i) const {
			b |= bitboard(right) << (i * 20);
			sc += score_r;
		}

        static bool can_combine(int& tile, int& hold){
            // 合并规则是两个值在数列中相邻或者同时为1的时候可以合并
            return (tile == 1 && hold == 1) || abs(tile - hold) == 1;
        }

		static int mvleft(uint32_t row[]) {
			int top = 0;
			int tmp = 0;
			int score = 0;

			for (int i = 0; i < 4; i++) {
				int tile = row[i];
				if (tile == 0) continue;
				row[i] = 0;
				if (tmp != 0) {
					if (can_combine(tile, tmp)) {
						tile = (tile > tmp) ? tile : tmp;
						row[top++] = ++tile;
						score += (fibonacci[tile]);
						tmp = 0;
					} else {
						row[top++] = tmp;
						tmp = tile;
					}
				} else {
					tmp = tile;
				}
			}
			if (tmp != 0) row[top] = tmp;
			return score;
		}

		lookup() {
			static int row = 0;
			init(row++);
		}

		static const lookup& find(const int& row) {
            // 这里列举了所有行可能的情况，只会初始化一次
			static const lookup cache[0xfffff];
			return cache[row];
		}
	};

public:
	/**
	 * apply an action to the board
	 * return the reward gained by the action, or -1 if the action is illegal
	 */

	int move(const int& opcode) {
		switch (opcode) {
		case 0: return move_up();
		case 1: return move_right();
		case 2: return move_down();
		case 3: return move_left();
		default: return -1;
		}
	}

	int move_left() {
		bitboard move = 0;
		bitboard prev = *this;
		int score = 0;
		lookup::find(fetch(0)).move_left(move, score, 0);
		lookup::find(fetch(1)).move_left(move, score, 1);
		lookup::find(fetch(2)).move_left(move, score, 2);
		lookup::find(fetch(3)).move_left(move, score, 3);
		*this = move;
		return (move != prev) ? score : -1;
	}

	int move_right() {
		bitboard move = 0;
		bitboard prev = *this;
		int score = 0;
		lookup::find(fetch(0)).move_right(move, score, 0);
		lookup::find(fetch(1)).move_right(move, score, 1);
		lookup::find(fetch(2)).move_right(move, score, 2);
		lookup::find(fetch(3)).move_right(move, score, 3);
		*this = move;
		return (move != prev) ? score : -1;
	}
	int move_up() {
		rotate_right();
		int score = move_right();
		rotate_left();
		return score;
	}
	int move_down() {
		rotate_right();
		int score = move_left();
		rotate_left();
		return score;
	}

    /**
     * 下面三个函数都参考了2048的bitboard变化
     * https://github.com/moporgic/TDL2048-Demo/blob/master/2048.cpp
     * 在2048的这个实作中每个tile占4bit(2^4 = 16)，三格和六格分别要移12和24位
     * 这份2584的代码中每个tile占5bit(2^5 = 32)，三格和六格分别要移15和30位
     * 
     * 类似bitboard(0xf83e, 0x7c1ff83e007c1f)这些数字是之前2048的代码中0xf0f00f0ff0f00f0fULL这样的数字变化得到的
     * 可以把这个数字看成一个蒙版(Mask)，具体可以通过下面的python代码转换：
     * 
        masks = [
            "f0f00f0ff0f00f0f",
            "0000f0f00000f0f0",
            "0f0f00000f0f0000",
            "ff00ff0000ff00ff",
            "00000000ff00ff00",
            "00ff00ff00000000"
        ]

        # results = []
        for mask in masks:
            result = ""
            for char in mask:
                # print(char)
                if char == 'f':
                    result += "11111"
                elif char == '0':
                    result += '00000'
                else:
                    raise ValueError()
            print(result[:16] + " " + result[16:])
            print(hex(int(result[:16],2)), hex(int(result[16:],2)))
     * 
     */

	/**
	 * swap row and column
	 * +------------------------+
     * |     1     3     7     2|
     * |     3     5     6     8|
     * |     1     2     5     7|
     * |     2     1     3     4|
     * +------------------------+
     * 
     * 变成下面这种
     * +------------------------+
     * |     1     3     1     2|
     * |     3     5     2     1|
     * |     7     6     5     3|
     * |     2     8     7     4|
     * +------------------------+
     * 
     * 
	 */
	void transpose() {
        bitboard result = *this;
		result = (result & bitboard(0xf83e, 0x7c1ff83e007c1f)) | ((result & bitboard(0x0, 0xf83e000000f83e0)) << 15) | ((result & bitboard(0x7c1, 0xf0000007c1f00000)) >> 15);
		result = (result & bitboard(0xffc0, 0xffc00003ff003ff)) | ((result & bitboard(0x0, 0xffc00ffc00)) << 30) | ((result & bitboard(0x3f, 0xf003ff0000000000)) >> 30);
        *this = result;
	}

    /**
	 * horizontal reflection
	 * +------------------------+       +------------------------+
	 * |     1     3     7     2|       |     2     7     3     1|
	 * |     3     5     6     8|       |     8     6     5     3|
	 * |     1     2     5     7| ----> |     7     5     2     1|
	 * |     2     1     3     4|       |     4     3     1     2|
	 * +------------------------+       +------------------------+
	 */
	void reflect_horizontal() {
        bitboard result = *this;
        result = ((result & bitboard(1, 0xf0001f0001f0001f)) << 15) | ((result & bitboard(0x3e, 0x3e0003e0003e0)) << 5)
            | ((result & bitboard(0x7c0, 0x7c0007c0007c00)) >> 5) | ((result & bitboard(0xf800, 0xf8000f8000f8000)) >> 15);
        *this = result;
		// for (int r = 0; r < 4; r++) {
		// 	std::swap(tile[r][0], tile[r][3]);
		// 	std::swap(tile[r][1], tile[r][2]);
		// }
	}

	/**
	 * vertical reflection
	 * +------------------------+       +------------------------+
	 * |     1     3     7     2|       |     2     1     3     4|
	 * |     3     5     6     8|       |     1     2     5     7|
	 * |     1     2     5     7| ----> |     3     5     6     8|
	 * |     2     1     3     4|       |     1     3     7     2|
	 * +------------------------+       +------------------------+
	 */
	void reflect_vertical() {
        bitboard result = *this;
        result = ((result & bitboard(0, 0xfffff)) << 60) | ((result & bitboard(0, 0xfffff00000)) << 20)
			| ((result & bitboard(0, 0xfffff0000000000)) >> 20) | ((result & bitboard(0xffff, 0xf000000000000000)) >> 60);
        *this = result;
		// for (int c = 0; c < 4; c++) {
		// 	std::swap(tile[0][c], tile[3][c]);
		// 	std::swap(tile[1][c], tile[2][c]);
		// }
	}

	/**
	 * rotate the board clockwise by given times
	 */
	void rotate(const int& r = 1) {
		switch (((r % 4) + 4) % 4) {
		default:
		case 0: break;
		case 1: rotate_right(); break;
		case 2: reverse(); break;
		case 3: rotate_left(); break;
		}
	}

	void rotate_right() { transpose(); reflect_horizontal(); } // clockwise
	void rotate_left() { transpose(); reflect_vertical(); } // counterclockwise
	void reverse() { reflect_horizontal(); reflect_vertical(); }

public:
    friend std::ostream& operator <<(std::ostream& out, const bitboard& b) {
		char buff[32];
		out << "+------------------------+" << std::endl;
		for (int i = 0; i < 16; i += 4) {
			std::snprintf(buff, sizeof(buff), "|%6u%6u%6u%6u|",
                b.at(i + 0),
                b.at(i + 1),
                b.at(i + 2),
                b.at(i + 3));
				// (fibonacci[b.at(i + 0)]),
				// (fibonacci[b.at(i + 1)]),
				// (fibonacci[b.at(i + 2)]),
				// (fibonacci[b.at(i + 3)]));
			out << buff << std::endl;
		}
		out << "+------------------------+" << std::endl;
		return out;
	}

public:
	uint16_t _left;
    uint64_t _right;
};