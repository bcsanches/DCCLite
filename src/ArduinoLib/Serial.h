#pragma once

#include <string>

struct SerialImpl
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
		std::string m_strData;
		size_t		m_uPos = 0;
};

extern SerialImpl Serial;


