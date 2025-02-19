// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <Arduino.h>

#include "main.h"

#include "LocalDecoderManager.h"
#include "SensorDecoder.h"
#include "ServoTurnoutDecoder.h"

#include "Console.h"
#include "Storage.h"

constexpr auto MAX_ROUTES = 15;

static unsigned long g_uLastFrameTime = 0;

bool Console::Custom_ParseCommand(dcclite::StringView command)
{
	return false;
}

bool Storage::Custom_LoadModules(const Storage::Lump &lump, EpromStream &stream)
{
	return false;
}

void Storage::Custom_SaveModules(EpromStream &stream)
{

}

void setup()
{		
	Console::Init();
	LocalDecoderManager::Init(__TIME__, __DATE__);

	{
		//
		//CRIANDO UM DESVIO
		//
		auto turnout = LocalDecoderManager::CreateServoTurnout(
			dcclite::SRVT_INVERTED_OPERATION,									//flags - opções do servo
			{ 10 },																//pin - pino do servo
			10,																	//range - quantos graus movimenta
			20,																	//ticks - quantos milisegundos entre cada grau
			dcclite::NullPin,													//powerPin - pino para ligar / desligar o relé do servo (opcional)
			dcclite::NullPin													//frogPin - pino para ligar / desligar rele do frog (opcional)
		);

		//
		//CRIANDO UM SENSOR QUE VAI MONITORAR UMA ENTRADA DO ARDUINO
		//
		auto sensor1 = LocalDecoderManager::CreateSensor(
			dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED,	//flags						flags
			{ 9 },											//pin						pino do sensor
			0,												//activate Delay (msec)		espera para ligar (opcional)
			0												//deactivate Delay (msec)	espera para desligar (opcional)
		);

		//Cria um botao que alterna o desvio de um lado para o outro quando pressionado
		LocalDecoderManager::CreateButton(
			*sensor1, 										//sensor do botão
			*turnout, 										//desvio que ele opera
			LocalDecoderManager::kTOGGLE					//o que fazer quando botão apertado: toggle - alterna
		);

		//
		//CRIANDO OUTRO SENSOR QUE VAI MONITORAR UMA ENTRADA DO ARDUINO
		//

		auto sensor2 = LocalDecoderManager::CreateSensor(
			dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED,	//flags
			{ 11 },											//pin
			0,												//activate Delay (msec)
			0												//deactivate Delay (msec)
		);

		//Este botão apenas faz THROW no desvio
		LocalDecoderManager::CreateButton(*sensor2, *turnout, LocalDecoderManager::kTHROW);

		//
		//CRIANDO OUTRO SENSOR QUE VAI MONITORAR UMA ENTRADA DO ARDUINO
		//

		auto sensor3 = LocalDecoderManager::CreateSensor(
			dcclite::SNRD_PULL_UP | dcclite::SNRD_INVERTED,	//flags
			{ 12 },											//pin
			0,												//activate Delay (msec)
			0												//deactivate Delay (msec)
		);

		//Este botão apenas faz CLOSE no desvio
		LocalDecoderManager::CreateButton(*sensor3, *turnout, LocalDecoderManager::kCLOSE);
	}

	g_uLastFrameTime = millis();

	LocalDecoderManager::PostInit();
}

void loop() 
{		
	auto currentTime = millis();	

	auto ticks = currentTime - g_uLastFrameTime;
	if (ticks == 0)
		return;	

	LocalDecoderManager::Update(ticks);	
}
