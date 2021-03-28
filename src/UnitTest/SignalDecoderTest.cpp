#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <gmock/gmock-matchers.h>

#include <rapidjson/document.h>

#include "SignalDecoder.h"

#include "IDccLiteService.h"
#include "IDevice.h"

using testing::HasSubstr;

using namespace rapidjson;

class DeviceDecoderServicesMockup : public IDevice_DecoderServices
{
public:
	std::string_view GetDeviceName() const noexcept override
	{
		return "mockup";
	}
};

class DecoderServicesMockup : public IDccLite_DecoderServices
{
public:
	void Decoder_OnStateChanged(Decoder &decoder) override
	{
		//empty
	}
};

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

std::unique_ptr<SignalDecoder> CreateSignal(const char *json)
{
	DccAddress					address{ 1024 };
	
	Document d;
	d.Parse(json);

	return std::make_unique<SignalDecoder>( address, "test", g_DecoderServices, g_DeviceDecoderServices,  d );
}

class SignalTester
{
	public:
		SignalTester(std::unique_ptr<SignalDecoder> signal):
			m_Signal(std::move(signal))
		{
			//empty
		}

		const std::map<std::string, std::string> &GetHeads()
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

bool VectorHasStr(const std::vector<std::string> &vec, const char *str)
{
	return std::any_of(vec.begin(), vec.end(), [str](auto &it) {return it.compare(str) == 0; });
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
				"on":["red"]
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
				"comment":"This is not parsed, this aspect is used for testing special cases on parsing"
			}      
		]  
	}
	)JSON";

	SignalTester tester{ CreateSignal(json) };

	ASSERT_EQ(tester.GetHeads().size(), 4);	

	auto &heads = tester.GetHeads();

	ASSERT_TRUE(heads.find("red") != heads.end());
	ASSERT_TRUE(heads.find("green") != heads.end());
	ASSERT_TRUE(heads.find("yellow") != heads.end());
	ASSERT_TRUE(heads.find("caution") != heads.end());

	ASSERT_STREQ(heads.find("red")->second.c_str(), "STC_HR12");
	ASSERT_STREQ(heads.find("green")->second.c_str(), "STC_HG12");
	ASSERT_STREQ(heads.find("yellow")->second.c_str(), "STC_HY12");
	ASSERT_STREQ(heads.find("caution")->second.c_str(), "STC_BLA");

	auto &aspects = tester.GetAspects();

	ASSERT_EQ(aspects.size(), 4);
	ASSERT_EQ(aspects[0].m_eAspect, dcclite::SignalAspects::Dark);
	ASSERT_EQ(aspects[1].m_eAspect, dcclite::SignalAspects::Clear);
	ASSERT_EQ(aspects[2].m_eAspect, dcclite::SignalAspects::Aproach);
	ASSERT_EQ(aspects[3].m_eAspect, dcclite::SignalAspects::Stop);

	for (auto &aspect : aspects)
	{
		ASSERT_FALSE(aspect.m_Flash);
	}

	//DARK aspect
	ASSERT_EQ(aspects[0].m_vecOnHeads.size(), 2);
	ASSERT_EQ(aspects[0].m_vecOffHeads.size(), 1);

	for (int i = 1; i < 3; ++i)
	{		
		ASSERT_EQ(aspects[i].m_vecOnHeads.size(), 1);
		ASSERT_EQ(aspects[i].m_vecOffHeads.size(), 3);
	}

	ASSERT_STREQ(aspects[1].m_vecOnHeads[0].c_str(), "green");
	ASSERT_STREQ(aspects[2].m_vecOnHeads[0].c_str(), "yellow");
	ASSERT_STREQ(aspects[3].m_vecOnHeads[0].c_str(), "red");

	ASSERT_TRUE(VectorHasStr(aspects[0].m_vecOffHeads, "green"));
	ASSERT_TRUE(VectorHasStr(aspects[0].m_vecOnHeads, "yellow"));
	ASSERT_TRUE(VectorHasStr(aspects[0].m_vecOnHeads, "caution"));

	ASSERT_TRUE(VectorHasStr(aspects[1].m_vecOffHeads, "red"));
	ASSERT_TRUE(VectorHasStr(aspects[1].m_vecOffHeads, "yellow"));

	ASSERT_TRUE(VectorHasStr(aspects[2].m_vecOffHeads, "red"));
	ASSERT_TRUE(VectorHasStr(aspects[2].m_vecOffHeads, "green"));

	ASSERT_TRUE(VectorHasStr(aspects[3].m_vecOffHeads, "yellow"));
	ASSERT_TRUE(VectorHasStr(aspects[3].m_vecOffHeads, "green"));
}

std::string ExtractSignalExceptionString(const char *json)
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

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("Error: expected heads object for"));

	json = R"JSON(
		{
			"name":"STC_SIG_12",
			"class":"VirtualSignal",    
			"address":"1840",
			"heads":1	
		}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("Error: expected heads object for"));
}

TEST(SignalDecoderTest, NoAspectsData)
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

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("Error: expected aspects array for"));

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
			"aspects":1
		}
	)JSON";

	ASSERT_THAT(ExtractSignalExceptionString(json), HasSubstr("Error: expected aspects array for"));
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
	)JSON"));
};
