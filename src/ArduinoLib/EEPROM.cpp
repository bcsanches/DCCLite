#include "EEPROM.h"

EEPROMImpl EEPROM;

#define EEPROM_SIZE 2048

static std::array<unsigned char, EEPROM_SIZE> g_Data;

void EEPROMImpl::get(size_t pos, void *ptr, size_t len)
{
	if (ptr == nullptr) 
	{
		throw std::invalid_argument("destination cannot be null");
	}

	if (pos + len > EEPROM_SIZE)
	{
		throw std::out_of_range("out of bounds");
	}

	memcpy(ptr, &g_Data[pos], len);
}

void EEPROMImpl::put(size_t pos, const void *ptr, size_t len)
{
	if (ptr == nullptr)
	{
		throw std::invalid_argument("destination cannot be null");
	}

	if (pos + len > EEPROM_SIZE)
	{
		throw std::out_of_range("out of bounds");
	}

	memcpy(&g_Data[pos], ptr, len);
}

unsigned char EEPROMImpl::read(size_t pos)
{
	return g_Data.at(pos);
}

size_t EEPROMImpl::length()
{
	return g_Data.max_size();
}

namespace ArduinoLib::detail
{
	void RomClear()
	{
		memset(&g_Data[0], 0, g_Data.size());
	}

	bool TrySaveRomState(const char *fileName)
	{
		FILE *fp = fopen(fileName, "wb");
		if (fp == nullptr)
			return false;

		return true;
	}

	bool TryLoadRomState(const char *fileName)
	{
		return true;
	}
}
