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
#include "tycoon/Industry.h"

#include "sys/Project.h"

#include "Helpers.h"

using namespace dcclite::broker::tycoon;

TEST(TycoonCycleStateTest, BasicCycle)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":60,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[				
				{
					"name":"Fechado Comum",
					"description":"Vagão fechado comum (genérico)",
					"AAR":"B",
					"cargos":[						
						"Produtos"
					]
				}
			],
			"locations":[
				{
					"name":"TC",
					"prefix":"TC",
					"industries":		
					[			
						{
							"name":"Entreposto",					
							"spot":"Gate",
							"dailyProduction":12,
							"maximumStorage":2,
							"produces": [
								{
									"cargo":"Produtos",
									"chance":50,
									"transferTimeHours":1,
									"destinations":["Lavras"]							
								}
							]
						}
					]
				}
			]
		}
	)JSON";

	auto tycoon = LoadTycoon(json, true /* delete state file if it exists*/);

	auto industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	dcclite::RName gateName{ "Gate" };

	industry->ReserveSpot(gateName, "bla");

	dcclite::RName cargoName{ "Produtos" };

	CheckException([gateName, industry, cargoName]
		{
			industry->StartSpotLoad(gateName, cargoName);
		},
		"[Tycoon::CargoInfo::StartCargoTransfer] No cargo in stock!!!"
	);

	Tick(120);	

	auto cargoQuantity = industry->GetCargoQuantity(cargoName);

	ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	industry->StartSpotLoad(gateName, cargoName);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);
	
	//spend some time...
	Tick(59);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);

	//finish transfer...
	Tick(1);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	//produce another item
	Tick(60);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	//produce another item
	Tick(120);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	ASSERT_FALSE(industry->IsProducing());

	//just spend some time...
	Tick(120);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	ASSERT_FALSE(industry->IsProducing());

	industry->RemoveCarFromSpot(gateName);

	for (int i = 0; i < 2; ++i)
	{
		//reserve again
		industry->ReserveSpot(gateName, "bla");

		//load again
		industry->StartSpotLoad(gateName, cargoName);

		//finish transfer...
		Tick(60);		

		industry->RemoveCarFromSpot(gateName);
	}	

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
}

/// <summary>
/// Checks if production restarts after a item is loaded...
/// </summary>
TEST(TycoonCycleStateTest, ProductionRestart)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":60,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[				
				{
					"name":"Fechado Comum",
					"description":"Vagão fechado comum (genérico)",
					"AAR":"B",
					"cargos":[						
						"Produtos"
					]
				}
			],
			"locations":[
				{
					"name":"TC",
					"prefix":"TC",
					"industries":		
					[			
						{
							"name":"Entreposto",					
							"spots":["Gate 1", "Gate 2"],
							"dailyProduction":12,
							"maximumStorage":2,
							"produces": [
								{
									"cargo":"Produtos",
									"chance":50,
									"transferTimeHours":1,
									"destinations":["Lavras"]							
								}
							]
						}
					]
				}
			]
		}
	)JSON";

	auto tycoon = LoadTycoon(json, true /* delete state file if it exists*/);

	auto industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	dcclite::RName gateName{ "Gate 1" };

	industry->ReserveSpot(gateName, "bla");

	Tick(240);

	dcclite::RName cargoName{ "Produtos" };
	auto cargoQuantity = industry->GetCargoQuantity(cargoName);

	ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	//production halted... reached max quantity.
	ASSERT_FALSE(industry->IsProducing());

	industry->StartSpotLoad(gateName, cargoName);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);	

	//load it...
	Tick(60);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	//production resumed...
	ASSERT_TRUE(industry->IsProducing());

	//produce another item
	Tick(120);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	ASSERT_FALSE(industry->IsProducing());

	industry->RemoveCarFromSpot(gateName);

	//
	//consume both at same time
	dcclite::RName gate2Name{ "Gate 2" };

	industry->ReserveSpot(gateName, "bla");
	industry->ReserveSpot(gate2Name, "bla");

	industry->StartSpotLoad(gateName, cargoName);	

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);

	industry->StartSpotLoad(gate2Name, cargoName);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 2);

	Tick(60);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	ASSERT_TRUE(industry->IsProducing());
}

TEST(TycoonCycleStateTest, InvalidStates)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":60,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[				
				{
					"name":"Fechado Comum",
					"description":"Vagão fechado comum (genérico)",
					"AAR":"B",
					"cargos":[						
						"Produtos"
					]
				}
			],
			"locations":[
				{
					"name":"TC",
					"prefix":"TC",
					"industries":		
					[			
						{
							"name":"Entreposto",					
							"spot":"Gate",
							"dailyProduction":12,
							"maximumStorage":2,
							"produces": [
								{
									"cargo":"Produtos",
									"chance":50,
									"transferTimeHours":1,
									"destinations":["Lavras"]							
								}
							]
						}
					]
				}
			]
		}
	)JSON";

	auto tycoon = LoadTycoon(json, true /* delete state file if it exists*/);

	auto industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	const dcclite::RName gateName{ "Gate" };
	const dcclite::RName cargoName{ "Produtos" };

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Initial state
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	//Cancel non reservation
	CheckException([industry, gateName]
		{
			industry->CancelSpotReservation(gateName);
		},
		"[Spot::CancelReservation] Spot is not reserved to cancel reservation"
	);

	//cannot load, spot not reserved
	CheckException([gateName, industry, cargoName]
		{
			industry->StartSpotLoad(gateName, cargoName);
		},
		"[Industry::StartSpotLoad] Spot Gate cannot be loaded because it is not reserved"
	);

	//cannot remove car...
	CheckException([gateName, industry, cargoName]
		{
			industry->RemoveCarFromSpot(gateName);
		},
		"[Spot::RemoveCar] Spot does not have a car parked to remove"
	);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Reserved State
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	industry->ReserveSpot(gateName, "bla");

	//Try to reserve again
	CheckException([industry, gateName]
		{
			industry->ReserveSpot(gateName, "bla");
		},
		"[Spot::Reserve] Spot is not free to reserve"
	);		

	//cannot load, no cargo
	CheckException([gateName, industry, cargoName]
		{
			industry->StartSpotLoad(gateName, cargoName);
		},
		"[Tycoon::CargoInfo::StartCargoTransfer] No cargo in stock!!!"
	);

	//cannot remove car...
	CheckException([gateName, industry, cargoName]
		{
			industry->RemoveCarFromSpot(gateName);
		},
		"[Spot::RemoveCar] Spot does not have a car parked to remove"
	);

	industry->CancelSpotReservation(gateName);

	//produce something...
	Tick(120);

	auto cargoQuantity = industry->GetCargoQuantity(cargoName);

	ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Load state
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	industry->ReserveSpot(gateName, "bla");
	industry->StartSpotLoad(gateName, cargoName);

	//cannot load, spot not reserved (loading...)
	CheckException([gateName, industry, cargoName]
		{
			industry->StartSpotLoad(gateName, cargoName);
		},
		"[Industry::StartSpotLoad] Spot Gate cannot be loaded because it is not reserved"
	);

	//Cancel non reservation
	CheckException([industry, gateName]
		{
			industry->CancelSpotReservation(gateName);
		},
		"[Spot::CancelReservation] Spot is not reserved to cancel reservation"
	);

	//cannot remove car...
	CheckException([gateName, industry, cargoName]
		{
			industry->RemoveCarFromSpot(gateName);
		},
		"[Spot::RemoveCar] Spot does not have a car parked to remove"
	);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);

	//finish transfer...
	Tick(60);	

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//
	// Park state
	//
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	
}