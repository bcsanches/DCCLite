#include <Arduino.h>

#include "SerialNet.h"
#include "Blinker.h"

static unsigned long g_startTime;
static unsigned long g_frameCount = 0;
static float g_fps = 0;

const int onboardLedPin = 13;

void setup()
{
	SerialNet::Init();
	Blinker::Init();	

	g_startTime = millis();

	Blinker::Play(Blinker::Animations::OK);
}

void loop() 
{
	Blinker::Update();
}
