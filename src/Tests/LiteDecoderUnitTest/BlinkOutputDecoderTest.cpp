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

#include <dcclite_shared/Packet.h>
#include <dcclite_shared/SharedLibDefs.h>

#include <exec/dcc/BlinkOutputDecoder.h>
#include <ArduinoLib.h>

#include <BlinkOutputDecoder.h>

#include "MiniDuino.h"
#include "../TestsCommon/BrokerMockups.h"

using namespace rapidjson;

namespace broker = dcclite::broker::exec::dcc;
using RemoteBlinkOutputDecoder = ::BlinkOutputDecoder;

static DecoderServicesMockup g_DecoderServices;
static DeviceDecoderServicesMockup g_DeviceDecoderServices;

static void WriteBlinkConfig(dcclite::Packet &packet, const char *json)
{
    Document document;
    document.Parse(json);

    broker::BlinkOutputDecoder output{
        broker::Address{1301},
        dcclite::RName{"test"},
        g_DecoderServices,
        g_DeviceDecoderServices,
        document
    };

    output.WriteConfig(packet);
}

TEST(LiteDecoder, BlinkOutputDecoder_InitialState)
{
    const char *json = R"JSON(
        {
            "name": "TEST_BLINK",
            "class": "BlinkOutput",
            "address": 1301,
            "pin": 45,
            "inverted": false,
            "ignoreSavedState": false,
            "location": "TC",
            "blinkInterval": 100
        }
    )JSON";

    dcclite::Packet packet;
    WriteBlinkConfig(packet, json);
    packet.Reset();

    ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_BLINK_OUTPUT));

    MiniDuino board;
    RemoteBlinkOutputDecoder decoder{ packet };

    ASSERT_EQ(decoder.GetType(), dcclite::DecoderTypes::DEC_BLINK_OUTPUT);

    ASSERT_FALSE(decoder.IsInverted());
    ASSERT_FALSE(decoder.IsActive());
    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

    decoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, millis());

    ASSERT_TRUE(decoder.IsActive());
    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);
}

TEST(LiteDecoder, BlinkOutputDecoder_TogglesAtInterval)
{
    const char *json = R"JSON(
        {
            "name": "TEST_BLINK",
            "class": "BlinkOutput",
            "address": 1301,
            "pin": 45,
            "inverted": false,
            "ignoreSavedState": true,
            "location": "TC",
            "blinkInterval": 100
        }
    )JSON";

    dcclite::Packet packet;
    WriteBlinkConfig(packet, json);

    packet.Reset();
    ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_BLINK_OUTPUT));

    MiniDuino board;
    RemoteBlinkOutputDecoder decoder{ packet };

    const unsigned long start = millis();

    decoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, start);

    decoder.SetBlinkingState(dcclite::DecoderStates::ACTIVE);

    // Antes do primeiro prazo, a saída permanece ligada.
    decoder.Update(start + 99);

    EXPECT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

    // Primeiro prazo: a fase inicial é sincronizada com ACTIVE.
    decoder.Update(start + 100);

    EXPECT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

    // Segundo prazo: a saída deve desligar.
    decoder.Update(start + 200);

    EXPECT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

    // Terceiro prazo: a saída deve ligar novamente.
    decoder.Update(start + 300);

    EXPECT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);
}

TEST(LiteDecoder, BlinkOutputDecoder_InactiveDecoderDoesNotToggle)
{
    const char *json = R"JSON(
        {
            "name": "TEST_BLINK",
            "class": "BlinkOutput",
            "address": 1301,
            "pin": 45,
            "inverted": false,
            "ignoreSavedState": true,
            "location": "TC",
            "blinkInterval": 100
        }
    )JSON";

    dcclite::Packet packet;
    WriteBlinkConfig(packet, json);
    packet.Reset();
    ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_BLINK_OUTPUT));

    MiniDuino board;
    RemoteBlinkOutputDecoder decoder{ packet };

    decoder.SetBlinkingState(dcclite::DecoderStates::ACTIVE);

    EXPECT_FALSE(decoder.IsActive());

    decoder.Update(millis() + 1000);

    EXPECT_FALSE(decoder.IsActive());
    EXPECT_EQ( ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);
}

TEST(LiteDecoder, BlinkOutputDecoder_InvertedOutput)
{
    const char *json = R"JSON(
        {
            "name": "TEST_BLINK",
            "class": "BlinkOutput",
            "address": 1301,
            "pin": 45,
            "inverted": true,
            "ignoreSavedState": true,
            "location": "TC",
            "blinkInterval": 100
        }
    )JSON";

    dcclite::Packet packet;
    WriteBlinkConfig(packet, json);
    packet.Reset();
    ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_BLINK_OUTPUT));

    MiniDuino board;
    RemoteBlinkOutputDecoder decoder{ packet };

    decoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, millis());

    EXPECT_TRUE(decoder.IsActive());

    EXPECT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);
}


TEST(LiteDecoder, BlinkOutputDecoder_StopBlinkingRestoresActiveOutput)
{
    const char *json = R"JSON(
        {
            "name": "TEST_BLINK",
            "class": "BlinkOutput",
            "address": 1301,
            "pin": 45,
            "inverted": false,
            "ignoreSavedState": true,
            "location": "TC",
            "blinkInterval": 100
        }
    )JSON";

    dcclite::Packet packet;
    WriteBlinkConfig(packet, json);
    packet.Reset();

    ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_BLINK_OUTPUT));

    MiniDuino board;
    RemoteBlinkOutputDecoder decoder{ packet };

    board.AddDecoder(decoder);

    const unsigned long start = millis();

    decoder.SetBlinkingState(dcclite::DecoderStates::ACTIVE);
    decoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, start);

    ASSERT_TRUE(decoder.IsActive());

    // Advance through two blink transitions until the physical output is LOW.
    decoder.Update(start + 100);
    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

    decoder.Update(start + 200);
    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

    // Stopping blinking must restore the output while the decoder is active.
    decoder.SetBlinkingState(dcclite::DecoderStates::INACTIVE);

    ASSERT_TRUE(decoder.IsActive());
    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);

    // The output must remain HIGH after blinking has stopped.
    decoder.Update(start + 300);
    decoder.Update(start + 400);

    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::HIGH);
}

TEST(LiteDecoder, BlinkOutputDecoder_StopBlinkingDoesNotActivateInactiveOutput)
{
    const char *json = R"JSON(
        {
            "name": "TEST_BLINK",
            "class": "BlinkOutput",
            "address": 1301,
            "pin": 45,
            "inverted": false,
            "ignoreSavedState": true,
            "location": "TC",
            "blinkInterval": 100
        }
    )JSON";

    dcclite::Packet packet;
    WriteBlinkConfig(packet, json);
    packet.Reset();

    ASSERT_EQ(packet.Read<uint8_t>(), static_cast<uint8_t>(dcclite::DecoderTypes::DEC_BLINK_OUTPUT));

    MiniDuino board;
    RemoteBlinkOutputDecoder decoder{ packet };

    board.AddDecoder(decoder);

    const unsigned long start = millis();

    decoder.SetBlinkingState(dcclite::DecoderStates::ACTIVE);
    decoder.AcceptServerState(dcclite::DecoderStates::ACTIVE, start);

    ASSERT_TRUE(decoder.IsActive());

    // Advance through two blink transitions until the physical output is LOW.
    decoder.Update(start + 100);
    decoder.Update(start + 200);

    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

    // Deactivate the decoder while the blinking output is LOW.
    decoder.AcceptServerState(dcclite::DecoderStates::INACTIVE, start + 200);

    ASSERT_FALSE(decoder.IsActive());
    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

    // Stopping blinking must not activate an inactive decoder.
    decoder.SetBlinkingState(dcclite::DecoderStates::INACTIVE);

    ASSERT_FALSE(decoder.IsActive());
    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);

    // The output must remain LOW after blinking has stopped.
    decoder.Update(start + 300);
    decoder.Update(start + 400);

    ASSERT_EQ(ArduinoLib::GetPinDigitalVoltage(45), VoltageModes::LOW);
}

