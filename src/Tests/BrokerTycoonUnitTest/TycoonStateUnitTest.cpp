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
					"name":"TC",
					"prefix":"TC",
					"industries":		
					[			
						{
							"name":"Entreposto",					
							"spot":"Gate",
							"dailyProduction":24,
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

	Tick(60);

	tycoon.reset();

	EXPECT_TRUE(dcclite::fs::exists(dcclite::broker::sys::Project::GetAppFilePath("tycoon.state.json")));

	//reloads with state file
	tycoon = LoadTycoon(json);

	industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	auto cargoQuantity = industry->GetCargoQuantity(cargoName);

	ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);

	industry->StartSpotLoad(gateName, cargoName);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);

	Tick(30);

	cargoQuantity = industry->GetCargoQuantity(cargoName);
	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);

	tycoon.reset();
}

TEST(TycoonServiceStateTest, RemovedSpot)
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
					"name":"TC",
					"prefix":"TC",
					"industries":		
					[			
						{
							"name":"Entreposto",					
							"spots":["Gate 1", "Gate 2"],
							"dailyProduction":24,
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

	dcclite::RName cargoName{ "Produtos" };

	CheckException([gateName, industry, cargoName]
		{
			industry->StartSpotLoad(gateName, cargoName);
		},
		"[Tycoon::CargoInfo::StartCargoTransfer] No cargo in stock!!!"
	);

	Tick(60);

	tycoon.reset();

	EXPECT_TRUE(dcclite::fs::exists(dcclite::broker::sys::Project::GetAppFilePath("tycoon.state.json")));

	const char *json2 = R"JSON(
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
					"name":"TC",
					"prefix":"TC",
					"industries":		
					[			
						{
							"name":"Entreposto",					
							"spots":["Gate 1"],
							"dailyProduction":24,
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

	//reloads with state file
	tycoon = LoadTycoon(json2);

	industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	auto cargoQuantity = industry->GetCargoQuantity(cargoName);

	ASSERT_EQ(cargoQuantity.m_uQuantity, 0);
	ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
}

TEST(TycoonServiceStateTest, MaxStorageChanges)
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
				}
			],
			"carTypes":[
				{
					"name":"Fechado Revestido",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Cimento Ensacado"
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
							"dailyProduction":24,
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

	Tick(120);

	ASSERT_FALSE(industry->IsProducing());

	dcclite::RName cargoName{ "Produtos" };

	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}

	tycoon.reset();

	EXPECT_TRUE(dcclite::fs::exists(dcclite::broker::sys::Project::GetAppFilePath("tycoon.state.json")));

	//
	//Increase maximumStorage, checks if production restarts....
	const char *json2 = R"JSON(
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
				}
			],
			"carTypes":[
				{
					"name":"Fechado Revestido",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Cimento Ensacado"
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
							"dailyProduction":24,
							"maximumStorage":4,
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

	//reloads with state file
	tycoon = LoadTycoon(json2);

	industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	ASSERT_TRUE(industry->IsProducing());

	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}

	Tick(120);

	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 4);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}

	ASSERT_FALSE(industry->IsProducing());

	//
	//Increase maximumStorage AGAIN
	json2 = R"JSON(
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
				}
			],
			"carTypes":[
				{
					"name":"Fechado Revestido",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Cimento Ensacado"
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
							"dailyProduction":24,
							"maximumStorage":6,
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

	tycoon.reset();
	
	tycoon = LoadTycoon(json2);

	industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	ASSERT_TRUE(industry->IsProducing());

	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 4);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}

	Tick(60);

	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 5);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}

	ASSERT_TRUE(industry->IsProducing());

	//
	//Now we decrease... check if production is not running after reload
	json2 = R"JSON(
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
				}
			],
			"carTypes":[
				{
					"name":"Fechado Revestido",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Cimento Ensacado"
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
							"dailyProduction":24,
							"maximumStorage":4,
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

	tycoon.reset();

	tycoon = LoadTycoon(json2);

	industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
	ASSERT_TRUE(industry);

	ASSERT_FALSE(industry->IsProducing());
	
	{
		//Quantity should stay...
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 5);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}

	//
	//
	//Now lets consume the cargo and check if production restarts
	//

	dcclite::RName gateName{ "Gate 1" };

	industry->ReserveSpot(gateName, "bla");
	industry->StartSpotLoad(gateName, cargoName);
	
	{
		//Quantity should update to reflect loading
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 4);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);
	}

	ASSERT_FALSE(industry->IsProducing());
	

	Tick(60);	

	//quantity should have decreased

	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 4);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}

	ASSERT_FALSE(industry->IsProducing());

	//still no production
	ASSERT_FALSE(industry->IsProducing());

	industry->RemoveCarFromSpot(gateName);

	industry->ReserveSpot(gateName, "bla");
	industry->StartSpotLoad(gateName, cargoName);
	

	//Quantity should update to reflect loading
	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 3);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);
	}

	ASSERT_FALSE(industry->IsProducing());

	Tick(60);

	//now we have room, production should restarts...
	ASSERT_TRUE(industry->IsProducing());

	{
		auto cargoQuantity = industry->GetCargoQuantity(cargoName);
		ASSERT_EQ(cargoQuantity.m_uQuantity, 3);
		ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
	}
}

TEST(TycoonServiceStateTest, LoadingState)
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
				}
			],
			"carTypes":[
				{
					"name":"Fechado Revestido",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Cimento Ensacado"
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
							"dailyProduction":24,
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

	dcclite::RName cargoName{ "Produtos" };
	dcclite::RName gateName{ "Gate 1" };

	{
		auto tycoon = LoadTycoon(json, true /* delete state file if it exists*/);

		auto industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
		ASSERT_TRUE(industry);

		Tick(120);

		ASSERT_FALSE(industry->IsProducing());		

		{
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
		}		

		industry->ReserveSpot(gateName, "bla");
		industry->StartSpotLoad(gateName, cargoName);

		{
			//Quantity should update to reflect loading
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);
		}

		Tick(30);

		{
			//still loading...
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);
		}
	}
	
	{
		//reload...
		auto tycoon = LoadTycoon(json);
		auto industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
		{
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 1);
		}

		//Try to reserve again
		CheckException([industry, gateName]
			{
				industry->ReserveSpot(gateName, "bla");
			},
			"[Spot::Reserve] Spot is not free to reserve"
		);

		//cannot load, spot not reserved (loading...)
		CheckException([gateName, industry, cargoName]
			{
				industry->StartSpotLoad(gateName, cargoName);
			},
			"[CargoProducer::StartSpotLoad] [Entreposto]: Spot Gate 1 cannot be loaded because it is not reserved"
		);

		//cannot remove car...
		CheckException([gateName, industry, cargoName]
			{
				industry->RemoveCarFromSpot(gateName);
			},
			"[Spot::RemoveCar] Spot does not have a car parked to remove"
		);

		ASSERT_FALSE(industry->IsProducing());

		//finish loading
		Tick(30);

		{
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
		}

		ASSERT_TRUE(industry->IsProducing());

		Tick(30);

		{
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
		}

		ASSERT_TRUE(industry->IsProducing());
	}

	{
		//reload...
		auto tycoon = LoadTycoon(json);
		auto industry = dynamic_cast<Industry *>(tycoon->TryNavigate(dcclite::ObjectPath{ "locations/TC/Entreposto" }));
		{
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 1);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
		}

		ASSERT_TRUE(industry->IsProducing());

		//Try to reserve again - car parked
		CheckException([industry, gateName]
			{
				industry->ReserveSpot(gateName, "bla");
			},
			"[Spot::Reserve] Spot is not free to reserve"
		);

		//cannot load, spot not reserved (car parked)
		CheckException([gateName, industry, cargoName]
			{
				industry->StartSpotLoad(gateName, cargoName);
			},
			"[CargoProducer::StartSpotLoad] [Entreposto]: Spot Gate 1 cannot be loaded because it is not reserved"
		);

		industry->RemoveCarFromSpot(gateName);

		//finish production
		Tick(30);

		{
			auto cargoQuantity = industry->GetCargoQuantity(cargoName);
			ASSERT_EQ(cargoQuantity.m_uQuantity, 2);
			ASSERT_EQ(cargoQuantity.m_uReservedQuantity, 0);
		}

		ASSERT_FALSE(industry->IsProducing());
	}
}