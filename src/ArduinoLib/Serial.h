#pragma once

#include <string>

#include "ArduinoLibDefs.h"

struct ARDUINO_API SerialImpl
{
	void begin(int frequency);

	void print(const char *str);
	void print(int value);
	void println(const char *str);

	int available();
	int read();

	void flush();

	inline void internalSetData(const char *data)
	{
		m_uPos = 0;
		m_strData.assign(data);
	}

	private:
#pragma warning(disable:4251)
		std::string m_strData;
#pragma warning(default:4251)

		size_t		m_uPos = 0;


};

ARDUINO_API extern SerialImpl Serial;


