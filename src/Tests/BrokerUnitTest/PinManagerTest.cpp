#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <rapidjson/document.h>

#include <dcclite_shared/BasicPin.h>

#include <dcclite/Object.h>
#include <dcclite/RName.h>

#include "exec/dcc/Address.h"
#include "exec/dcc/PinManager.h"
#include "exec/dcc/RemoteDecoder.h"

using namespace dcclite;
using namespace dcclite::broker::exec::dcc;

// Mock RemoteDecoder for testing
class MockRemoteDecoder : public RemoteDecoder
{
	public:
		MockRemoteDecoder(const Address &address)
			: RemoteDecoder(
				address, 
				RName("Mock"), 
				*(IDccLite_DecoderServices*)nullptr, 
				*(IDevice_DecoderServices*)nullptr, 
				rapidjson::Value{ rapidjson::kObjectType }
			)
		{
			//empty
		}
		
	MOCK_METHOD(dcclite::DecoderTypes, GetType, (), (const, noexcept, override));
	MOCK_METHOD(bool, IsOutputDecoder, (), (const, noexcept, override));
	MOCK_METHOD(bool, IsInputDecoder, (), (const, noexcept, override));
};

TEST(PinManagerTest, ConstructMegaAndUno) 
{
	PinManager mega(ArduinoBoards::MEGA);
	PinManager uno(ArduinoBoards::UNO);

	// Construction should not throw
}

TEST(PinManagerTest, RegisterAndUnregisterPin_Success) 
{
	PinManager manager(ArduinoBoards::UNO);

	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin pin(2);

	manager.RegisterPin(decoder, pin, "test");
	manager.UnregisterPin(decoder, pin);
}

TEST(PinManagerTest, RegisterPin_NullPin_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin nullPin;
	
	EXPECT_THROW(manager.RegisterPin(decoder, nullPin, "test"), std::invalid_argument);
}

TEST(PinManagerTest, RegisterPin_OutOfRange_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin pin(100);
	
	EXPECT_THROW(manager.RegisterPin(decoder, pin, "test"), std::out_of_range);
}

TEST(PinManagerTest, RegisterPin_ProtectedPin_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin pin(10); // UNO protected pin
	
	EXPECT_THROW(manager.RegisterPin(decoder, pin, "test"), std::invalid_argument);
}

TEST(PinManagerTest, RegisterPin_AlreadyUsed_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder1{ Address{1024} }, decoder2{ Address{1025} };
	dcclite::BasicPin pin(2);	

	manager.RegisterPin(decoder1, pin, "test");
	EXPECT_THROW(manager.RegisterPin(decoder2, pin, "test2"), std::invalid_argument);
}

TEST(PinManagerTest, RegisterPin_BadPin_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin pin(2);	

	// Mark as bad
	manager.MarkBadPin(pin);
	EXPECT_THROW(manager.RegisterPin(decoder, pin, "test"), std::invalid_argument);
}

TEST(PinManagerTest, UnregisterPin_NotRegistered_Throws)
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin pin(2);
	
	EXPECT_THROW(manager.UnregisterPin(decoder, pin), std::invalid_argument);
}

TEST(PinManagerTest, UnregisterPin_WrongUser_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder1{ Address{1024} }, decoder2{ Address{1025} };
	dcclite::BasicPin pin(2);	

	manager.RegisterPin(decoder1, pin, "test");
	EXPECT_THROW(manager.UnregisterPin(decoder2, pin), std::invalid_argument);
}

TEST(PinManagerTest, UnregisterPin_NullPin_NoThrow) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin nullPin;

	// Should not throw
	manager.UnregisterPin(decoder, nullPin);
}

TEST(PinManagerTest, UnregisterPin_OutOfRange_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	MockRemoteDecoder decoder{ Address{1024} };
	dcclite::BasicPin pin(100);
	
	EXPECT_THROW(manager.UnregisterPin(decoder, pin), std::out_of_range);
}

TEST(PinManagerTest, MarkBadPin_NullPin_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	dcclite::BasicPin nullPin;
	EXPECT_THROW(manager.MarkBadPin(nullPin), std::invalid_argument);
}

TEST(PinManagerTest, MarkBadPin_OutOfRange_Throws) 
{
	PinManager manager(ArduinoBoards::UNO);
	dcclite::BasicPin pin(100);
	EXPECT_THROW(manager.MarkBadPin(pin), std::out_of_range);
}

TEST(PinManagerTest, Serialize_CallsStream) 
{
	PinManager manager(ArduinoBoards::UNO);

	JsonCreator::StringWriter writer;
	JsonOutputStream_t stream{ writer };

	// Should not throw
	manager.Serialize(stream);
}