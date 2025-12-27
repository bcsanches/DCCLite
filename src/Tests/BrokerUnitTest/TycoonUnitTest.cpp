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

#include <rapidjson/document.h>

#include <dcclite/RName.h>

#include "sys/ServiceFactory.h"
#include "tycoon/TycoonService.h"

using namespace dcclite;
using namespace dcclite::broker::tycoon;
using namespace dcclite::broker::sys;
using namespace rapidjson;

static std::unique_ptr<TycoonService> LoadTycoon(const char *json)
{
	Document d;
	d.Parse(json);
	auto obj = d.GetObject();

	TycoonService::RegisterFactory();

	auto ptr = ServiceFactory::TryFindFactory(RName{ "TycoonService" })->Create(
		RName{ "tycoon" },
		*static_cast<Broker *>(nullptr),
		obj
	);

	return std::unique_ptr<TycoonService>{ static_cast<TycoonService *>(ptr.release()) };
}

static void CheckLoadException(const char *json, const char *expectedMessage)
{	
	try
	{
		LoadTycoon(json);
		
		FAIL() << "Expected exception!!";
	}
	catch (std::exception &ex)
	{
		EXPECT_STREQ(
			ex.what(),
			expectedMessage
		);
	}
}

TEST(TycoonServiceTest, Basic)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				},
				{
					"name":"Gado"			
				},
				{
					"name":"Conteiner"
				},
				{
					"name":"Bobinas"
				}
			]
		}	
	)JSON";

	LoadTycoon(json);
}

TEST(TycoonServiceTest, InvalidClockRate0)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":0			
		}	
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] fastClockRate must be greater than zero");	
}

TEST(TycoonServiceTest, InvalidClockRateNegative)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":-1			
		}	
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] fastClockRate must be greater than zero");
}

TEST(TycoonServiceTest, InvalidClockRateTooBig)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":256			
		}	
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] fastClockRate must be less or equal than 255");	
}

TEST(TycoonServiceTest, CargoDataIsNotArray)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos": "should fail"	
		}	
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: invalid cargos definition, expected array");
}

TEST(TycoonServiceTest, CargoArrayShouldContainObjects)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				},
				"name",
				{
					"name":"Conteiner"
				}
			]
		}	
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: invalid cargo definition, expected object");
}

TEST(TycoonServiceTest, CargoArrayShouldContainCargoNames)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				},				
				{
					"noNameForMe":"Conteiner"
				}
			]
		}	
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: cargo missing name property");	
}

TEST(TycoonServiceTest, CargoArrayWithDuplicates)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				},
				{
					"name":"Gado"			
				},
				{
					"name":"Conteiner"
				},
				{
					"name":"Gado"
				}
			]
		}	
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: duplicate cargo name 'Gado'");
}

TEST(TycoonServiceTest, CarTypeWithInvalidType)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				},
				{
					"name":"Gado"			
				},
				{
					"name":"Conteiner"
				}
			],
			"carTypes":"test"
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: invalid carTypes definition, expected array");
}

TEST(TycoonServiceTest, CarTypeWithEmptyArray)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				},
				{
					"name":"Gado"			
				},
				{
					"name":"Conteiner"
				}
			],
			"carTypes":[]
		}
	)JSON";

	LoadTycoon(json);	
}

TEST(TycoonServiceTest, CarTypeWithInvalidCarTypeEntry)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				},
				{
					"name":"Gado"			
				},
				{
					"name":"Conteiner"
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Produtos"
				},
				"invalid entry"
			]
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: invalid carType definition, expected object");
}

TEST(TycoonServiceTest, CarTypeWithoutName)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Produtos"
				}
			]
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: carType missing name property");
}

TEST(TycoonServiceTest, CarTypeDuplicated)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Produtos"
				},
				{
					"name":"Fechado",
					"ABNT":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Produtos"
				}
			]
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: duplicate carType name 'Fechado'");
}

TEST(TycoonServiceTest, CarTypeWithoutCode1)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"ABNT_INVALID":"FR",
					"description":"Vagão Fechado convencional caixa metálica com revestimento",
					"cargo":"Produtos"
				}
			]
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: carType 'Fechado' missing type property, must be ABNT or AAR");
}

TEST(TycoonServiceTest, CarTypeWithAAR)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"AAR":"FR",
					"cargo":"Produtos"
				}
			]
		}
	)JSON";

	LoadTycoon(json);
}

TEST(TycoonServiceTest, CarTypeWithoutCargo1)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"AAR":"FR"					
				}
			]
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: carType 'Fechado' has no cargos assigned");
}

TEST(TycoonServiceTest, CarTypeWithoutCargo2)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"AAR":"FR",
					"cargos":[]
				}
			]
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: carType 'Fechado' has no cargos assigned");
}

TEST(TycoonServiceTest, CarTypeWithInvalidCargosData)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"AAR":"FR",
					"cargos":"should be an array"			
				}
			]
		}
	)JSON";

	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [Load] error: carType 'Fechado' has invalid cargos definition, expected array");
}

TEST(TycoonServiceTest, CarTypeWithInvalidCargoName1)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"AAR":"FR",
					"cargo":"I DO NOT EXIST"
				}
			]
		}
	)JSON";

	//blow up as RNAME is not registered
	CheckLoadException(json, "[RName::GetName] Name \"I DO NOT EXIST\" is not registered");
}

TEST(TycoonServiceTest, CarTypeWithInvalidCargoName2)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"AAR":"FR",
					"cargo":"Fechado"
				}
			]
		}
	)JSON";

	//blow up as RNAME is exists, but no cargo
	CheckLoadException(json, "[TycoonServiceImpl::tycoon] [AddCargoToCarType] error: carType 'Fechado' references unknown cargo 'Fechado'");
}


TEST(TycoonServiceTest, CarTypeWithDuplicatedCargo)
{
	const char *json = R"JSON(
		{
			"class":"TycoonService",
			"name":"tycoon",
			"fastClockRate":4,
			"cargos":[
				{
					"name":"Produtos"			
				}
			],
			"carTypes":[
				{
					"name":"Fechado",
					"AAR":"FR",
					"cargos":["Produtos", "Produtos"]
				}
			]
		}
	)JSON";

	//blow up as RNAME is exists, but no cargo
	CheckLoadException(json, "[CarType::Fechado] Cargo 'Produtos' is already added to car type");
}
