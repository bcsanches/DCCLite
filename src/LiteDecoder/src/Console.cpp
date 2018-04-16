#include "Console.h"

#include <stdarg.h>
#include <stdio.h>

#include <Arduino.h>

#include "LiteDecoder.h"


void Console::Init()
{
    Serial.begin(9600);
    Serial.flush();

    Serial.print("LiteDecoder ");
    Serial.print(ARDUINO_TYPE);
    Serial.print(" / ");    
    Serial.print(VERSION);
    Serial.print(" / ");
    Serial.print(__DATE__);
    Serial.print(" ");
    Serial.println(__TIME__);    
}

void Console::SendLog(const char *module, const char *format, ...)
{
    char buffer[128];

    va_list args;
    va_start(args, format);

    vsnprintf(buffer, 128, format, args);

    Serial.println("");
    Serial.print("<LOG ");
    Serial.print(module);
    Serial.print(" ");

    Serial.print(buffer);

    Serial.println(">");

    va_end(args);
}

void Console::Send(const char *str)
{
    Serial.print(str);
}

void Console::SendLn(const char *str)
{
    Serial.println(str);
}

void Console::Send(int value)
{
    Serial.print(value);
}

int Console::Available()
{
    return Serial.available();
}

int Console::ReadChar()
{
    return Serial.read();
}
