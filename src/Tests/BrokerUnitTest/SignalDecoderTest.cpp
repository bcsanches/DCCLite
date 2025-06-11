#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include <rapidjson/document.h>

#include <dcclite/Log.h>

#include "exec/dcc/SignalDecoder.h"


using testing::HasSubstr;

using namespace rapidjson;

#include "../TestsCommon/BrokerMockups.h"

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

using namespace dcclite::broker::exec::dcc;

std::unique_ptr<SignalDecoder> CreateSignal(const char *json)
{
	Address					address{ 1024 };
	
	Document d;
	d.Parse(json);

	return std::make_unique<SignalDecoder>(address, dcclite::RName{ "test" }, g_DecoderServices, g_DeviceDecoderServices, d);
}

class SignalTester
{
	public:
		explicit SignalTester(std::unique_ptr<SignalDecoder> signal):
			m_Signal(std::move(signal))
		{
			//empty
		}

		const std::map<dcclite::RName, dcclite::RName> &GetHeads()
		{
			return m_Signal->m_mapHeads;
		}

		const std::vector<SignalDecoder::Aspect> &GetAspects()
		{
			return m_Signal->m_vecAspects;
		}

	private:
		std::unique_ptr<SignalDecoder> m_Signal;
};

bool VectorHasStr(const std::vector<dcclite::RName> &vec, const char *str)
{
	return std::any_of(vec.begin(), vec.end(), [str](auto &it) {return it.GetData().compare(str) == 0; });
}

TEST(SignalDecoderTest, Basic)
{
	const char *json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",  
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"yellow":"STC_HY12",
				"caution":"STC_BLA"
			},        
			"aspects":
			[
			{
				"name":"Stop",
				"on":["red"],
				"flash":false,
				"comment":"Flash false is the default, we just put it here for testing"				
			},
			{
				"name":"Clear",
				"on":["green"]
			},
			{
				"name":"Aproach",
				"on":["yellow"]		
			},
			{
				"name":"Dark",
				"on":["yellow", "caution"],
				"off":["green"],
				"flash":true,
				"comment":"This is not parsed, this aspect is used for testing special cases on parsing"
			}      
		]  
	}
	)JSON";	

	SignalTester tester{ CreateSignal(json) };

	ASSERT_EQ(tester.GetHeads().size(), 4);	

	auto &heads = tester.GetHeads();

	ASSERT_TRUE(heads.find(dcclite::RName{ "red" }) != heads.end());
	ASSERT_TRUE(heads.find(dcclite::RName{ "green" }) != heads.end());
	ASSERT_TRUE(heads.find(dcclite::RName{ "yellow" }) != heads.end());
	ASSERT_TRUE(heads.find(dcclite::RName{ "caution" }) != heads.end());

	ASSERT_STREQ(heads.find(dcclite::RName{ "red" })->second.GetData().data(), "STC_HR12");
	ASSERT_STREQ(heads.find(dcclite::RName{ "green" })->second.GetData().data(), "STC_HG12");
	ASSERT_STREQ(heads.find(dcclite::RName{ "yellow" })->second.GetData().data(), "STC_HY12");
	ASSERT_STREQ(heads.find(dcclite::RName{ "caution" })->second.GetData().data(), "STC_BLA");

	auto &aspects = tester.GetAspects();

	ASSERT_EQ(aspects.size(), 4);
	ASSERT_EQ(aspects[0].m_kAspect, dcclite::SignalAspects::Dark);
	ASSERT_EQ(aspects[1].m_kAspect, dcclite::SignalAspects::Clear);
	ASSERT_EQ(aspects[2].m_kAspect, dcclite::SignalAspects::Aproach);
	ASSERT_EQ(aspects[3].m_kAspect, dcclite::SignalAspects::Stop);

	//
	// 	   
	//DARK aspect
	//
	//
	ASSERT_EQ(aspects[0].m_vecOnHeads.size(), 2);
	ASSERT_EQ(aspects[0].m_vecOffHeads.size(), 1) << "m_vecOffHeads should contain one element only";
	ASSERT_TRUE(aspects[0].m_Flash);

	//Check if m_vecOffHeads on DARK aspect contains only green, that is configured on JSON	
	ASSERT_TRUE(VectorHasStr(aspects[0].m_vecOffHeads, "STC_HG12"));

	ASSERT_TRUE(VectorHasStr(aspects[0].m_vecOnHeads, "STC_HY12"));
	ASSERT_TRUE(VectorHasStr(aspects[0].m_vecOnHeads, "STC_BLA"));

	//
	//
	// Other Aspects
	//
	//
	for (int i = 1; i < 3; ++i)
	{		
		ASSERT_EQ(aspects[i].m_vecOnHeads.size(), 1);
		ASSERT_EQ(aspects[i].m_vecOffHeads.size(), 3);
		ASSERT_FALSE(aspects[i].m_Flash);
	}

	//size was checked to be 1, so just confirm correct head is there
	ASSERT_STREQ(aspects[1].m_vecOnHeads[0].GetData().data(), "STC_HG12");
	ASSERT_STREQ(aspects[2].m_vecOnHeads[0].GetData().data(), "STC_HY12");
	ASSERT_STREQ(aspects[3].m_vecOnHeads[0].GetData().data(), "STC_HR12");

	//size was checked to be 3, so just confirm the heads are there
	ASSERT_TRUE(VectorHasStr(aspects[1].m_vecOffHeads, "STC_HR12"));
	ASSERT_TRUE(VectorHasStr(aspects[1].m_vecOffHeads, "STC_HY12"));
	ASSERT_TRUE(VectorHasStr(aspects[1].m_vecOffHeads, "STC_BLA"));

	ASSERT_TRUE(VectorHasStr(aspects[2].m_vecOffHeads, "STC_HR12"));
	ASSERT_TRUE(VectorHasStr(aspects[2].m_vecOffHeads, "STC_HG12"));
	ASSERT_TRUE(VectorHasStr(aspects[2].m_vecOffHeads, "STC_BLA"));

	ASSERT_TRUE(VectorHasStr(aspects[3].m_vecOffHeads, "STC_HY12"));
	ASSERT_TRUE(VectorHasStr(aspects[3].m_vecOffHeads, "STC_HG12"));
	ASSERT_TRUE(VectorHasStr(aspects[3].m_vecOffHeads, "STC_BLA"));	
}

static std::string ExtractSignalExceptionString(const char *json)
{	
	try
	{
		CreateSignal(json);		
	}	
	catch (std::exception &ex)
	{
		return ex.what();
	}

	throw std::exception("GTEST FAILURE");
}

TEST(SignalDecoderTest, NoHeadsData)
{
	auto json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840"		
		}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("[SignalDecoder] Error: expected heads object"));
}

TEST(SignalDecoderTest, HeadsDataIsNotObject)
{
	auto json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",
			"heads":1	
		}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("[SignalDecoder] Error: heads data is not an object")) << "Heads data in JSON is not an object";
}

TEST(SignalDecoderTest, Aspects_EmptyArray)
{
	const char *json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",  
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12"			
			},        
			"aspects":
			[			
			]  
		}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr(" [SignalDecoder] Error: aspects array is empty"));
};


TEST(SignalDecoderTest, Aspects_DataIsNotArray)
{
	auto json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"yellow":"STC_HY12",
				"caution":"STC_BLA"
			},
			"aspects":1
		}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("[SignalDecoder] Error: expected aspects data to be an array"));
}

TEST(SignalDecoderTest, Aspects_NoData)
{
	auto json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"yellow":"STC_HY12",
				"caution":"STC_BLA"
			}
		}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("[SignalDecoder] Error: expected aspects array"));
}


TEST(SignalDecoderTest, DuplicatedAspect)
{
	const char *json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",  
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"yellow":"STC_HY12",
				"caution":"STC_BLA"
			},        
			"aspects":
			[
			{
				"name":"Stop",
				"on":["red"]
			},
			{
				"name":"Stop",
				"on":["green"]
			}
		]  
	}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("already defined"));
}

TEST(SignalDecoderTest, MissingHeads)
{
	const char *json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",  
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"yellow":"STC_HY12",
				"caution":"STC_BLA"
			},        
			"aspects":
			[
			{
				"name":"Stop",
				"on":["NULL"]
			},
			{
				"name":"Clear",
				"on":["green"]
			}
		]  
	}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("on \"on\" array not found on heads defintion"));

	//
	// Check Off Array
	//

	json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",  
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"yellow":"STC_HY12",
				"caution":"STC_BLA"
			},        
			"aspects":
			[
			{
				"name":"Stop",
				"on":["red"]
			},
			{
				"name":"Clear",
				"on":["green"],
				"off":["NULL"]
			}
		]  
	}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("on \"off\" array not found on heads defintion"));
}

TEST(SignalDecoderTest, HeadsBothOnOff)
{
	const char *json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",  
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"yellow":"STC_HY12",
				"caution":"STC_BLA"
			},        
			"aspects":
			[
			{
				"name":"Stop",
				"on":["red"],
				"off":["green", "red"]
			}
		]  
	}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("also defined on the \"on\" table"));
}

TEST(SignalDecoderTest, InvalidAspectName)
{	
	ASSERT_ANY_THROW(CreateSignal(R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",  
			"heads":
			{
				"red":"STC_HR12",
				"green":"STC_HG12",
				"BLA":"STC_HY12"
			},        
			"aspects":
			[
			{
				"name":"will not parse here",
				"on":["red"]
			}
		]  
	}
	)JSON")) << "Signal definition in JSON uses an name for aspect that is not know [BLA]";
};


