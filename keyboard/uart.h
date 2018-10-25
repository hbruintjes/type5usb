#pragma once

#include <stdint.h>

template<uint8_t Size>
class ring_buffer {
	static_assert(Size >= 2, "Ringbuffer too small");
public:
	static constexpr uint8_t size = Size;

	void push(uint8_t c) {
		buffer[w_pos] = c;
		w_pos = static_cast<uint8_t>(w_pos + 1) % Size;
	}

	uint8_t cur() {
		return buffer[r_pos];
	}

	uint8_t pop() {
		auto& c = buffer[r_pos];
		r_pos = static_cast<uint8_t>(r_pos + 1) % Size;
		return c;
	}

	uint8_t length() {
		if (r_pos > w_pos) {
			return Size - r_pos + w_pos;
		} else {
			return w_pos - r_pos;
		}
	}

	uint8_t free() {
		if (r_pos > w_pos) {
			return r_pos - w_pos;
		} else {
			return Size - w_pos + r_pos;
		}
	}

	bool full() const {
		return (w_pos + 1) % Size == r_pos;
	}

	bool empty() const {
		return w_pos == r_pos;
	}

	void clear() {
		r_pos = w_pos = 0;
	}

private:
	uint8_t buffer[Size] = {0};
	uint8_t r_pos = 0;
	uint8_t w_pos = 0;
};

namespace uart {
	void init(uint16_t baudrate, uint8_t sampling = 32u);

	bool poll();

	bool full();

	uint8_t recv();

	bool ready();
	void clear();

	bool send(uint8_t c);
	bool send(uint8_t c1, uint8_t c2);
}