#include "EEPROM.h"

EEPROMImpl EEPROM;

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

	memcpy(ptr, &m_Data[pos], len);
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

	memcpy(&m_Data[pos], ptr, len);
}

unsigned char EEPROMImpl::read(size_t pos)
{
	return m_Data.at(pos);
}

size_t EEPROMImpl::length()
{
	return m_Data.max_size();
}
