#pragma once

#include <array>

#define EEPROM_SIZE 2048

struct EEPROMImpl
{
	void put(size_t pos, const void *ptr, size_t len);
	void get(size_t pos, void *ptr, size_t len);

	template<typename T>
	inline void put(size_t pos, T &data)
	{
		put(pos, &data, sizeof(T));
	}

	template <typename T>
	inline void get(size_t pos, T &data)
	{
		get(pos, &data, sizeof(T));
	}

	size_t length();

	unsigned char read(size_t pos);

	private:
		std::array<unsigned char, EEPROM_SIZE> m_Data;
};

extern EEPROMImpl EEPROM;

