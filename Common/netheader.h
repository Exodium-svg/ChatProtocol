#pragma once
#include "pch.h"
#include <stdint.h>

#pragma pack(push, 1)  // Set the packing alignment to 1 byte
struct DLL_SPEC NET_MESSAGE {
	const uint64_t length;
	const uint32_t flags;
	const uint16_t id;
	const uint8_t checksum;

	NET_MESSAGE(uint64_t length, uint16_t id, uint32_t flags);
	NET_MESSAGE(uint64_t length, uint32_t flags, uint16_t id);

	inline const uint8_t calculateChecksum() const {
		uint8_t localChecksum = 0;

		// XOR the ID and length values together
		localChecksum ^= static_cast<uint8_t>(id);     // ID is a field in the structure
		localChecksum ^= static_cast<uint8_t>(length); // length is a field in the structure

		return localChecksum;
	}
};
#pragma pack(pop)  // Restore the default alignment