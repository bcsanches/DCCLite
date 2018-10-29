#include <stdio.h>

#include "Arduino.h"

extern void loop();
extern void setup();

int main(int, char **)
{
	printf("hello\n");

#ifdef _DEBUG
	ArduinoLib::Setup("LiteDecoderLib_d.dll");
#else
	ArduinoLib::Setup("LiteDecoderLib.dll");
#endif

	ArduinoLib::Tick();

	ArduinoLib::SetSerialInput("<*C OUTD 1 7 0>");

	ArduinoLib::Tick();

	ArduinoLib::SetSerialInput("<*C TRGR 2 6 1 T>");

	ArduinoLib::Tick();

	ArduinoLib::SetSerialInput("<*C TRGR 3 5 1 C>");		

	ArduinoLib::Tick();

	ArduinoLib::SetPinDigitalVoltage(6, LOW);	

	ArduinoLib::Tick();	

	ArduinoLib::SetPinDigitalVoltage(5, LOW);

	ArduinoLib::Tick();

	ArduinoLib::SetPinDigitalVoltage(6, HIGH);

	ArduinoLib::Tick();

	ArduinoLib::SetPinDigitalVoltage(5, HIGH);

	ArduinoLib::Tick();

	return 0;
}

