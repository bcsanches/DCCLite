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

bool Console::Custom_ParseCommand(const char *command)
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

struct RouteNode
{
	int m_iTurnoutIndex;
	dcclite::DecoderStates m_kState;
};


static RouteNode g_stRoutes[] = {
	//route 0
	{0, dcclite::DecoderStates::INACTIVE},
	{1, dcclite::DecoderStates::INACTIVE},
	{-1, dcclite::DecoderStates::INACTIVE},

	//route 1
	{0, dcclite::DecoderStates::INACTIVE},
	{1, dcclite::DecoderStates::ACTIVE},
	{-1, dcclite::DecoderStates::INACTIVE},

	//end
	{-1, dcclite::DecoderStates::INACTIVE}
};	

static RouteNode *g_pstRoutes[MAX_ROUTES] = { nullptr };

class RouteManager
{
	public:
		RouteManager():
			m_iCurrentRoute{ 0 },
			m_iCurrentRouteNode{ 0 },
			m_iNextRoute{ -1 }
		{
			//empty
		}		

		void Init()
		{
			//try to find which route is set
			for (int i = 0; i < MAX_ROUTES; ++i)
			{
				RouteNode *node;
				
				for (node = g_pstRoutes[i]; node->m_iTurnoutIndex >= 0; ++node)
				{
					auto turnout = static_cast<ServoTurnoutDecoder *>(LocalDecoderManager::TryGetDecoder(node->m_iTurnoutIndex));
					if (turnout->GetDecoderState() != node->m_kState)
						break;
				}

				//found end?
				if (node->m_iTurnoutIndex < 0)
				{
					m_iCurrentRoute = i;
					m_iCurrentRouteNode = 0;

					break;
				}
			}
		}

		void Update()
		{
			//Current route set and next route set?
			if ((g_pstRoutes[m_iCurrentRoute][m_iCurrentRouteNode].m_iTurnoutIndex < 0) && (m_iNextRoute >= 0))
			{				
				//Yes! Set to next route
				m_iCurrentRoute = m_iNextRoute;
				m_iCurrentRouteNode = 0;		

				//clear next route
				m_iNextRoute = -1;
			}

			auto &currentNode = g_pstRoutes[m_iCurrentRoute][m_iCurrentRouteNode];

			//Current route node is end?
			if (currentNode.m_iTurnoutIndex < 0)
				return;

			auto currentNodeTurnout = static_cast<ServoTurnoutDecoder *>(LocalDecoderManager::TryGetDecoder(currentNode.m_iTurnoutIndex));

			//Is it moving?
			if (currentNodeTurnout->IsMoving())
			{
				//wait it finishes
				return;
			}

			//Is turnout in desired position?
			if (currentNodeTurnout->GetDecoderState() != currentNode.m_kState)
			{
				//set position and check again
				currentNodeTurnout->AcceptServerState(currentNode.m_kState);

				return;
			}

			//Turnout is in current position, so next frame, set next turnout state
			++m_iCurrentRouteNode;
		}

		void SetNextRoute(int routeIndex)
		{
			m_iNextRoute = routeIndex;			
		}

	private:
		int m_iCurrentRoute;
		int m_iCurrentRouteNode;

		int m_iNextRoute;
};

bool g_fStable = false;
RouteManager g_clRouteManager;

SensorDecoder *g_Sensor;

void setup()
{		
	Console::Init();

	LocalDecoderManager::CreateServoTurnout(
		0,			//flags
		{ 10 },		//pin
		15,			//range
		10,			//ticks
		{ 11 },		//powerPin
		{ 12 }		//frogPin
	);

	g_Sensor = LocalDecoderManager::CreateSensor(
		0,
		{9},
		0,
		0
	);

	int baseIndex = 0;
	for (int i = 0; ; ++i)
	{
		g_pstRoutes[baseIndex++] = g_stRoutes + i;

		//skip current turnout
		++i;

		//search for end marker
		while (g_stRoutes[i].m_iTurnoutIndex >= 0)
			++i;

		//advance
		++i;

		//end marker? (double -1)
		if (g_stRoutes[i].m_iTurnoutIndex < 0)
			break;
	}

	g_uLastFrameTime = millis();
}

void loop() 
{		
	auto currentTime = millis();	

	auto ticks = currentTime - g_uLastFrameTime;
	if (ticks == 0)
		return;	

	LocalDecoderManager::Update(ticks);	

	if(g_Sensor->IsActive())
	{
		Console::Send("Hello");
	}

#if 0
	if (!g_fStable)
	{
		for (int i = 0; i < MAX_DECODERS; ++i)
		{
			if (g_pclTurnouts[i]->IsMoving())
				return;
		}

		g_fStable = true;
		g_clRouteManager.Init();
	}

	g_clRouteManager.Update();
#endif
}
