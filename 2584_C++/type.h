#pragma once
#include <iostream>
#include <string>

class state_type {
public:
	enum type : char {
		before = 'b',
		after = 'a',
		illegal = 'i'
	};

public:
	state_type() : t(illegal) {}
	state_type(const state_type& st) = default;
	state_type(state_type::type code) : t(code) {}

	friend std::istream& operator >>(std::istream& in, state_type& type) {
		std::string s;
		if (in >> s) type.t = static_cast<state_type::type>((s + " ").front());
		return in;
	}

	friend std::ostream& operator <<(std::ostream& out, const state_type& type) {
		switch (type.t) {
		case state_type::before:  return out << "before";
		case state_type::after:   return out << "after";
		case state_type::illegal: return out << "illegal";
		default:                  return out << "unknown";
		}
	}

	bool is_before()  const { return t == before; }
	bool is_after()   const { return t == after; }
	bool is_illegal() const { return t == illegal; }

private:
	type t;
};
