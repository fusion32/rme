//////////////////////////////////////////////////////////////////////
// This file is part of Remere's Map Editor
//////////////////////////////////////////////////////////////////////
// Remere's Map Editor is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// Remere's Map Editor is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.
//////////////////////////////////////////////////////////////////////

#ifndef RME_POSITION_H_
#define RME_POSITION_H_

#include <stdint.h>
#include <iostream>

#include "const.h"

struct Position {
	int x, y, z;

	Position() : x(0), y(0), z(0) {}
	Position(int x, int y, int z) : x(x), y(y), z(z) {}

	bool operator<(const Position &other) const noexcept {
		return z < other.z && y < other.y && x < other.x;
	}

	bool operator>(const Position &other) const noexcept {
		return z > other.z && y > other.y && x > other.x;
	}

	Position operator-(const Position &other) const noexcept {
		return Position(x - other.x, y - other.y, z - other.z);
	}

	Position operator+(const Position &other) const noexcept {
		return Position(x + other.x, y + other.y, z + other.z);
	}

	Position &operator+=(const Position &other) {
		*this = *this + other;
		return *this;
	}

	bool operator==(const Position& other) const noexcept {
		return z == other.z && y == other.y && x == other.x;
	}

	bool operator!=(const Position& other) const noexcept {
		return !(*this == other);
	}

	bool isValid() const noexcept {
		if(x == 0 && y == 0 && z == 0)
			return false;
		return (z >= rme::MapMinLayer && z <= rme::MapMaxLayer)
			&& (y >= 0                && y <= rme::MapMaxHeight)
			&& (x >= 0                && x <= rme::MapMaxWidth);
	}
};

inline std::ostream &operator<<(std::ostream &os, const Position &pos) {
	return os << "(" << pos.x << "," << pos.y << "," << pos.z << ")";
}

inline std::istream &operator>>(std::istream &is, Position &pos) {
	char ch;
	is >> ch; if(ch != '(') return is;
	is >> pos.x;
	is >> ch; if(ch != ',') return is;
	is >> pos.y;
	is >> ch; if(ch != ',') return is;
	is >> pos.z;
	is >> ch; if(ch != ')') return is;
	return is;
}

inline int PackAbsoluteCoordinate(Position pos){
	// DOMAIN: [24576, 40959] x [24576, 40959] x [0, 15]
	// TODO(fusion): Warning if position is outside domain?
	int packed = (((pos.x - 24576) & 0x00003FFF) << 18)
				| (((pos.y - 24576) & 0x00003FFF) << 4)
				| (pos.z & 0x0000000F);
	return packed;
}

inline Position UnpackAbsoluteCoordinate(int packed){
	Position pos;
	pos.x = ((packed >> 18) & 0x00003FFF) + 24576;
	pos.y = ((packed >>  4) & 0x00003FFF) + 24576;
	pos.z = ((packed >>  0) & 0x0000000F);
	return pos;
}

inline int PackRelativeCoordinate(Position pos){
	// DOMAIN: [-8192, 8191] x [-8192, 8191] x [-8, 7]
	// TODO(fusion): Warning if position is outside domain?
	int packed = (((pos.x + 8192) & 0x00003FFF) << 18)
				| (((pos.y + 8192) & 0x00003FFF) << 4)
				| ((pos.z + 8) & 0x0000000F);
	return packed;
}

inline Position UnpackRelativeCoordinate(int packed){
	Position pos;
	pos.x = ((packed >> 18) & 0x00003FFF) - 8192;
	pos.y = ((packed >>  4) & 0x00003FFF) - 8192;
	pos.z = ((packed >>  0) & 0x0000000F) - 8;
	return pos;
}

#endif //RME_POSITION_H_
