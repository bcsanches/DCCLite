#include "Serial.h"

#include <stdio.h>

SerialImpl Serial;

void SerialImpl::begin(int frequency)
{
	//nothing todo...
}

void SerialImpl::print(const char *str)
{
	printf("%s", str);
}

void SerialImpl::print(int value, int base)
{
	printf(base == 10 ? "%d" : "%X", value);
}

void SerialImpl::print(unsigned int value, int base)
{
	printf(base == 10 ? "%u" : "%X", value);
}

void SerialImpl::print(char value)
{
	printf("%c", value);
}

void SerialImpl::println(const char *str)
{
	printf("%s\n", str);
}

int SerialImpl::available()
{
	return m_strData.length() - m_uPos;
}

int SerialImpl::read()
{		
	return m_uPos >= m_strData.length() ? -1 : m_strData[m_uPos++];
}

void SerialImpl::flush()
{
	//empty
}
