#include <Arduino.h>

#include "Blinker.h"
#include "Console.h"
#include "NetUdp.h"
#include "Storage.h"

static unsigned long g_uStartTime = 0;
static unsigned long g_uFrameCount = 0;
static float g_uFps = 0;

const int onboardLedPin = 13;

void setup()
{
	Console::Init();
	Blinker::Init();

	Storage::LoadConfig();

	NetUdp::Init();	

	g_uStartTime = millis();

	Blinker::Play(Blinker::Animations::OK);	

	Console::SendLog("setup", "done");
}

void loop() 
{	
	++g_uFrameCount;

	auto currentTime = millis();
	int seconds = 0;

	while((currentTime - g_uStartTime) >= 1000)
	{
		++seconds;
		g_uStartTime += 1000;
	}

	if(seconds > 0)
	{
		g_uFps = g_uFrameCount / static_cast<float>(seconds);
		g_uFrameCount = 0;

		//Console::sendLog("main", "fps %d", (int)g_fps);
	}

	Console::Update();
	Blinker::Update();
}
