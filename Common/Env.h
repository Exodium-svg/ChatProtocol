#pragma once
#include <vector>
#include "Stream.h"
#include "FileSystem.h"

enum DLL_SPEC VarType: uint16_t {
	VAR_STRING,
	VAR_INT,
	VAR_BOOL,
	VAR_INVALID,
};

struct DLL_SPEC EnvVar {
	char* key;
	VarType type;
	uint32_t length;
	void* value;
	// Very improper, however I am not sure how else to fix it, Just make sure to never modify the key directly as it is shared memory.
	EnvVar(const char* key, const VarType type, uint32_t length, void* value): key((char*)key), type(type), length(length), value(value) {}
	EnvVar() : key(nullptr), type(VAR_INVALID), length(0), value(nullptr) {};
	~EnvVar() {
		// Illegal, but I do not care.
		free((void*)key);
		free(value);
	}
};

class DLL_SPEC Env
{
	std::vector<EnvVar> m_vVars;

public:
	Env(Stream& stream);
	Env(const FilePath& path);
	Env() {}

	std::string GetString(const char* key, std::string fallback) const;
	int32_t GetInt(const char* key, int32_t fallback) const;
	bool GetBool(const char* key, bool fallback) const;

	void ReadFromStream(Stream& stream);
	void SaveToStream(Stream& stream);
};

