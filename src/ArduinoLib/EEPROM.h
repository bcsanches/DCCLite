#pragma once

#include <array>

#include "ArduinoLibDefs.h"

struct ARDUINO_API EEPROMImpl
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
};

ARDUINO_API extern EEPROMImpl EEPROM;




