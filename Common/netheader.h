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

	inline const uint8_t calculateChecksum(const void* pBuff, const uint64_t size) const {
		uint8_t localChecksum = 0;

		for (uint64_t i = 0; size > i; i++)
			localChecksum += reinterpret_cast<const char*>(pBuff)[i];
		

		return localChecksum;
	}
};
#pragma pack(pop)  // Restore the default alignment