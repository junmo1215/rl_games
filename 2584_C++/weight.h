#pragma once
#include <iostream>
#include <vector>
#include <array>
#include <sstream>
#include <iterator>
#include <string>

/**
 * weight table of n-tuple network
 */
class weight {
public:
	weight() : length(0), value(nullptr) {}
	weight(const size_t& len) : length(len), value(alloc(len)) {}
	weight(weight&& f) : length(f.length), value(f.value) { f.value = nullptr; }
	weight(const weight& f) = delete;
	weight& operator =(const weight& f) = delete;
	virtual ~weight() { delete[] value; }

	float& operator[] (const size_t& i) { return value[i]; }
	const float& operator[] (const size_t& i) const { return value[i]; }
	size_t size() const { return length; }

public:
	friend std::ostream& operator <<(std::ostream& out, const weight& w) {
		float* value = w.value;
		size_t size = w.size();
		out.write(reinterpret_cast<char*>(&size), sizeof(size_t));
		out.write(reinterpret_cast<char*>(value), sizeof(float) * size);
		return out;
	}

	friend std::istream& operator >>(std::istream& in, weight& w) {
		float*& value = w.value;
		size_t& size = w.length;
		if (value) {
			std::cerr << "reading to a non-empty weight" << std::endl;
			std::exit(1);
		}
		if (in.read(reinterpret_cast<char*>(&size), sizeof(size_t))) {
			value = alloc(size);
			in.read(reinterpret_cast<char*>(value), sizeof(float) * size);
		}
		if (!in) {
			std::cerr << "unexpected end of binary" << std::endl;
			std::exit(1);
		}
		return in;
	}

protected:
	static float* alloc(size_t num) {
		static size_t total = 0;
		static size_t limit = (2 << 30) / sizeof(float); // 2G memory
		try {
			total += num;
			if (total > limit) throw std::bad_alloc();
			return new float[num]();
		} catch (std::bad_alloc&) {
			std::cerr << "memory limit exceeded" << std::endl;
			std::exit(-1);
		}
		return nullptr;
	}

	size_t length;
	float* value;
};
