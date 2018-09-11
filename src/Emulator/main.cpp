#include <stdio.h>

#include "Arduino.h"

int main(int, char **)
{
	printf("hello\n");

	ArduinoLib::setup();

	ArduinoLib::tick();

	ArduinoLib::setSerialInput("<*C OUTD 1 7 0>");

	ArduinoLib::tick();

	ArduinoLib::setSerialInput("<*C TRGR 2 6 1 T>");

	ArduinoLib::tick();

	ArduinoLib::setSerialInput("<*C TRGR 3 5 1 C>");		

	ArduinoLib::tick();

	ArduinoLib::setPinDigitalVoltage(6, LOW);	

	ArduinoLib::tick();	

	ArduinoLib::setPinDigitalVoltage(5, LOW);

	ArduinoLib::tick();

	ArduinoLib::setPinDigitalVoltage(6, HIGH);

	ArduinoLib::tick();

	ArduinoLib::setPinDigitalVoltage(5, HIGH);

	ArduinoLib::tick();

	return 0;
}

