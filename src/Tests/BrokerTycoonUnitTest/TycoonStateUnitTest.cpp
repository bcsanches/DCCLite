// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <gtest/gtest.h>

#include "tycoon/TycoonService.h"

#include "sys/Project.h"

#include "Helpers.h"

static void Tick(int fastClockTicks)
{
	using namespace dcclite::broker::sys;
	
	for (int i = 0; i < fastClockTicks; ++i)
	{
		auto first = Thinker::TryGetFirstThinker();
		ASSERT_TRUE(first);

		dcclite::broker::sys::Thinker::UpdateThinkers(first->GetTimePoint());
	}
}

TEST(TycoonServiceStateTest, BasicState)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":60,
			"cargos":[
				{
					"name":"Produtos"			
				},
				{
					"name":"Cimento Ensacado"			
				},
				{
					"name":"Conteiner"
				},
				{
					"name":"Bobinas"
				}
			],
			"carTypes":[
				{
					"name":"Fechado Revestido",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Cimento Ensacado"
				},
				{
					"name":"Fechado Comum",
					"description":"Vagão fechado comum (genérico)",
					"AAR":"B",
					"cargos":[
						"Cimento Ensacado",
						"Produtos"
					]
				}
			],
			"locations":[
				{
					"name":"Três Corações",
					"prefix":"TC",
					"industries":		
					[			
						{
							"name":"Entreposto SPR",					
							"spot":"Portão 01",
							"dailyProduction":24,
							"maximumStorage":2,
							"produces": [
								{
									"cargo":"Produtos",
									"chance":50,
									"transferTimeHours":4,
									"destinations":["Lavras"]							
								}
							]
						}
					]
				}
			]
		}	
	)JSON";

	auto tycoon = LoadTycoon(json, true /* delete state file if exists*/);		

	Tick(60);

	tycoon.reset();

	EXPECT_TRUE(dcclite::fs::exists(dcclite::broker::sys::Project::GetAppFilePath("tycoon.state.json")));

	//reloads with state file
	tycoon = LoadTycoon(json);
}
