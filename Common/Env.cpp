#include "pch.h"
#include "Env.h"

constexpr int defaultSize = 50;

Env::Env(Stream& stream): m_vVars(std::vector<EnvVar>(defaultSize)) { ReadFromStream(stream); }

Env::Env(const FilePath& path)
{
	if (FileSystem::Exists(path)) {
		FileStream fileStream = FileSystem::Read(path);
		ReadFromStream(*&fileStream);
	}
}

std::string Env::GetString(const char* key, std::string fallback) const
{
	for (int32_t i = 0; i < m_vVars.size(); i++) {
		EnvVar var = m_vVars[i];

		if (strcmp(key, var.key)) {
			if (!var.type == VarType::VAR_STRING)
				throw std::runtime_error("Value taken is not a string");

			return std::move(std::string(reinterpret_cast<char*>(var.value)));
		}
	}

	return fallback;
}

int32_t Env::GetInt(const char* key, int32_t fallback) const
{
	for (int32_t i = 0; i < m_vVars.size(); i++) {
		EnvVar var = m_vVars[i];

		if (strcmp(key, var.key)) {
			if (!var.type == VarType::VAR_INT)
				throw std::runtime_error("Value taken is not a string");

			return *reinterpret_cast<int32_t*>(var.value);
		}
	}

	return fallback;
}

bool Env::GetBool(const char* key, bool fallback) const
{
	for (int32_t i = 0; i < m_vVars.size(); i++) {
		EnvVar var = m_vVars[i];

		if (strcmp(key, var.key)) {
			if (!var.type == VarType::VAR_BOOL)
				throw std::runtime_error("Value taken is not a string");

			return *reinterpret_cast<bool*>(var.value);
		}
	}

	return fallback;
}

void Env::ReadFromStream(Stream& stream)
{
	uint32_t count;
	stream >> count;

	if (m_vVars.size() < defaultSize)
		m_vVars.resize(count);

	for (uint32_t i = 0; i < count; i++) {
		uint16_t strLen;
		stream >> strLen;

		char* key = reinterpret_cast<char*>(malloc(strLen));
		size_t readBytes = stream.Read(key, strLen);

		if (readBytes != strLen)
			throw std::runtime_error("Was unable to read all the bytes from the key");

		uint16_t varType;
		stream >> varType;

		uint32_t valueSize;
		stream >> valueSize;

		void* pValue = malloc(valueSize);

		readBytes = stream.Read(pValue, valueSize);

		if (readBytes != valueSize)
			throw std::runtime_error("Was unable to read all bytes from the value");

		m_vVars.emplace_back(key, static_cast<VarType>(varType), valueSize, pValue);
	}
}

void Env::SaveToStream(Stream& stream)
{
	stream << (uint32_t)m_vVars.size();

	for (uint32_t i = 0; i < m_vVars.size(); i++) {
		EnvVar var = m_vVars[i];
		uint16_t nStrLen = strlen(var.key);

		stream << nStrLen;
		size_t nWrittenBytes = stream.Write(var.key, nStrLen);

		if(nWrittenBytes != nStrLen)
			throw std::runtime_error("Was unable to write all the bytes from the key");

		stream << static_cast<uint16_t>(var.type);
		stream << var.length;

		nWrittenBytes = stream.Write(var.value, var.length);

		if (nWrittenBytes != var.length)
			throw std::runtime_error("Was unable to write all bytes from the value");
	}
}
