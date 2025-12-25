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

	Document d;
	d.Parse(json);

	auto obj = d.GetObject();

	TycoonService::RegisterFactory();

	ServiceFactory::TryFindFactory(RName{ "TycoonService" })->Create(
		RName{ "tycoon" },
		*static_cast<Broker *>(nullptr),
		obj
	);
}

static void CheckLoadException(const char *json, const char *expectedMessage)
{
	Document d;
	d.Parse(json);
	auto obj = d.GetObject();

	TycoonService::RegisterFactory();
	try
	{
		ServiceFactory::TryFindFactory(RName{ "TycoonService" })->Create(
			RName{ "tycoon" },
			*static_cast<Broker *>(nullptr),
			obj
		);
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
