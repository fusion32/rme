#ifndef RME_BITSET_H_
#define RME_BITSET_H_ 1

#include <stdint.h>

template<int N>
struct BitSet {
	uint8_t data[(N + 7) / 8];

	bool check(int index) const {
		if(index < 0 || index >= N){
			return false;
		}

		int     byteIndex = (index / 8);
		uint8_t bitMask   = (uint8_t)(1 << (index % 8));
		return (this->data[byteIndex] & bitMask) != 0;
	}

	void set(int index) {
		if(index < 0 || index >= N){
			return;
		}

		int     byteIndex = (index / 8);
		uint8_t bitMask   = (uint8_t)(1 << (index % 8));
		this->data[byteIndex] |= bitMask;
	}

	void clear(int index) {
		if(index < 0 || index >= N){
			return;
		}

		int     byteIndex = (index / 8);
		uint8_t bitMask   = (uint8_t)(1 << (index % 8));
		this->data[byteIndex] &= ~bitMask;
	}
};

#endif //RME_BITSET_H_
