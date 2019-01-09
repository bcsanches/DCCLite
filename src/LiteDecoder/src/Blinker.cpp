#include "Blinker.h"

#include <assert.h>
#include <Arduino.h>

#define LOCAL_LED 13

#define SLOW 1000
#define FAST 300
#define MAX_FRAMES 3

static unsigned long g_iNextThink = 0;
static short g_iAnimation = -1;
static uint8_t g_iVoltage = 0;
static char g_iFrame = 0;

#define MAX_ANIMATIONS 2

static const uint8_t g_uAnimations[MAX_ANIMATIONS] = {
	(1 << 0) | (1 << 1) | (1 << 2), //OK
	(0 << 0) | (0 << 1) | (0 << 2), //ERROR
};

static void StartAnimation(uint8_t animation)
{
	assert(animation < MAX_ANIMATIONS);

    g_iAnimation = animation;
	g_iFrame = -1;
    g_iVoltage = LOW;    

	g_iNextThink = millis() + FAST;

    digitalWrite(LOCAL_LED, g_iVoltage);    
}

void Blinker::Play(Animations animation)
{
	StartAnimation(static_cast<uint8_t>(animation));
}

void Blinker::Init()
{
	//initialize digital pin 13 as an output.
    pinMode(LOCAL_LED, OUTPUT);
	digitalWrite(LOCAL_LED, LOW);
}

void Blinker::Update()
{
	if(g_iAnimation < 0)
		return;

    unsigned long ticks = millis();
    if(g_iNextThink <= ticks)
    {
		if(g_iVoltage == LOW)
		{
			++g_iFrame;
			if(g_iFrame == MAX_FRAMES)
			{
				g_iAnimation = -1;
				return;
			}

			g_iVoltage = HIGH;			
		}
		else
		{
			g_iVoltage = LOW;			
		}
		g_iNextThink = ticks + (bitRead(g_uAnimations[g_iAnimation], g_iFrame) ? FAST : SLOW);

#if 0
		Serial.println("bit / think / frame");
		Serial.print(bitRead(g_uAnimations[g_iAnimation], g_iFrame));
		Serial.print("  ");
		Serial.print(g_iNextThink);
		Serial.print("  ");
		Serial.println(g_iFrame);
#endif

        digitalWrite(LOCAL_LED, g_iVoltage);
    }
}
