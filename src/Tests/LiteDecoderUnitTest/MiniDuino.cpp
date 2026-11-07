#include "MiniDuino.h"

#include <assert.h>

#include <rapidjson/document.h>

#include <Arduino.h>
#include <ArduinoLib.h>

#include <dcclite/JsonUtils.h>

#include <dcclite_shared/Packet.h>

#include <exec/dcc/BlinkOutputDecoder.h>
#include <exec/dcc/BlinkOutputMasterDecoder.h>

#include <Decoder.h>
#include <BlinkOutputDecoder.h>
#include <BlinkOutputMasterDecoder.h>

#include "../TestsCommon/BrokerMockups.h"

static MiniDuino *g_pclSingleton = nullptr;

namespace DecoderManager
{	
	Decoder *TryGetDecoder(const uint8_t slot) noexcept
	{
		assert(g_pclSingleton);

		return g_pclSingleton->GetDecoder(slot);
	}	
}

MiniDuino::MiniDuino()
{
	assert(!g_pclSingleton);

	g_pclSingleton = this;

	ArduinoLib::Setup(Setup, Loop, "UnitTestMiniDuino");
}

MiniDuino::~MiniDuino()
{
	ArduinoLib::Finalize();

	g_pclSingleton = nullptr;
}

void MiniDuino::AddDecoder(Decoder &dec)
{
	m_vecDecoders.push_back(&dec);
}

Decoder *MiniDuino::GetDecoder(uint8_t index)
{
	return m_vecDecoders.at(index);
}

void MiniDuino::Loop()
{
	auto ticks = millis();

	for (auto dec : g_pclSingleton->m_vecDecoders)
	{
		dec->Update(ticks);
	}
}

ManagedMiniDuino::~ManagedMiniDuino()
{
	for (auto dec : m_vecDecoders)
	{
		delete dec;
	}
}

void ManagedMiniDuino::Load(const char *json)
{
	using namespace rapidjson;
	namespace broker = dcclite::broker::exec::dcc;

	Document d;
	d.Parse(json);

	auto ar = d.GetArray();

	std::vector<std::unique_ptr<broker::RemoteDecoder>> defs;

	DecoderServicesMockup decoderServices;
	DeviceDecoderServicesMockup deviceDecoderServices;

	for (auto it = ar.begin(); it != ar.end(); ++it)
	{		
		dcclite::RName name{ dcclite::json::GetString(*it, "name") };
		auto address = broker::Address{ static_cast<uint16_t>((*it)["address"].GetUint()) };
		auto className = dcclite::json::GetString(*it, "class");

		if (className.compare("BlinkOutputMaster") == 0)
		{
			defs.push_back(std::make_unique<broker::BlinkOutputMasterDecoder>(address, name, decoderServices, deviceDecoderServices, *it));
		}
		else if (className.compare("BlinkOutput") == 0)
		{
			defs.push_back(std::make_unique<broker::BlinkOutputDecoder>(address, name, decoderServices, deviceDecoderServices, *it));
		}
		else
		{
			//blowup
			assert(0);
		}

		deviceDecoderServices.RegisterDecoder(*defs.back());
	}

	for (auto &it : defs)
	{
		it->InitAfterDeviceLoad();
	}

	using RemoteBlinkOutputDecoder = ::BlinkOutputDecoder;
	using RemoteBlinkOutputMasterDecoder = ::BlinkOutputMasterDecoder;

	for (auto &it : defs)
	{	
		dcclite::Packet packet;

		it->WriteConfig(packet);

		packet.Reset();
		switch (static_cast<dcclite::DecoderTypes>(packet.Read<uint8_t>()))
		{
			case dcclite::DecoderTypes::DEC_BLINK_OUTPUT_MASTER:
				{
					auto *master = new BlinkOutputMasterDecoder(packet);
					this->AddDecoder(*master);
				}
				break;

			case dcclite::DecoderTypes::DEC_BLINK_OUTPUT:
				{
					auto *slave = new BlinkOutputDecoder(packet);
					this->AddDecoder(*slave);
				}
				break;

			default:
				assert(0);
				break;
		}
	}
}

void ManagedMiniDuino::Store()
{
	Storage::EpromStream stream{ 0 };

	stream.Put(uint16_t{ 0xB0 });
	stream.Put(uint16_t{ 0x0B });

	assert(m_vecDecoders.size() <= std::numeric_limits<uint8_t>::max());

	stream.Put(static_cast<uint8_t>(m_vecDecoders.size()));

	for (auto it : m_vecDecoders)
	{
		stream.Put(static_cast<uint8_t>(it->GetType()));
		it->SaveConfig(stream);
	}
}

void ManagedMiniDuino::LoadFromStorage()
{
	assert(m_vecDecoders.size() == 0);

	Storage::EpromStream stream{ 0 };

	//validate magic
	assert(stream.Read<uint16_t>() == 0xB0);
	assert(stream.Read<uint16_t>() == 0x0B);	

	//num of decoders
	auto count = stream.Read<uint8_t>();
	if (count == 0)
		return;

	m_vecDecoders.reserve(count);

	for (auto i = 0; i < count; ++i)
	{
		auto type = static_cast<dcclite::DecoderTypes>(stream.Read<uint8_t>());
		switch (type)
		{
			case dcclite::DecoderTypes::DEC_BLINK_OUTPUT_MASTER:
			{
				auto *master = new BlinkOutputMasterDecoder(stream);
				this->AddDecoder(*master);
			}
			break;

			case dcclite::DecoderTypes::DEC_BLINK_OUTPUT:
			{
				auto *slave = new BlinkOutputDecoder(stream);
				this->AddDecoder(*slave);
			}
			break;

			default:
				assert(0);
				break;
		}
	}
}
