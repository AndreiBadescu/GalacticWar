#pragma once

#include <cstdint>

namespace gal {
	enum FunctionId : uint8_t {
		GIVE_TURN,
		SURRENDER,
		CAPTURE,
		MOVE,
		ATTACK,
		BUY,
		CORRUPTED_PACKET
	};

	struct Packet {
		uint8_t size;
		FunctionId function_id;
		char data[100];
	};
}