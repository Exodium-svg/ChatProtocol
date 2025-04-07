#include "pch.h"
#include "netheader.h"

NET_MESSAGE::NET_MESSAGE(uint64_t length, uint16_t id, uint32_t flags) : length(length), flags(flags), id(id), checksum(calculateChecksum(this, length)) {}

NET_MESSAGE::NET_MESSAGE(uint64_t length, uint32_t flags, uint16_t id) :length(length), flags(flags), id(id), checksum(calculateChecksum(this, length)) {}