// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "ServoProgrammer.h"

#include "Console.h"
#include "DecoderManager.h"
#include "Packet.h"
#include "ServoTurnoutDecoder.h"
#include "Session.h"

class ServoProgrammerHelper
{
	public:
		void SetDecoder(ServoTurnoutDecoder &decoder)
		{
			//enable programming mode
			decoder.PM_EnableProgMode();

			m_pclDecoder = &decoder;	
			m_uServoPos = m_pclDecoder->m_clServo.read();
		}

		ServoTurnoutDecoder *ClearDecoder()
		{
			assert(m_pclDecoder);
			
			m_pclDecoder->PM_DisableProgMode();

			auto *p = m_pclDecoder;

			m_pclDecoder = nullptr;

			return p;
		}

		inline bool IsSet() const noexcept
		{
			return m_pclDecoder;
		}		

		void MoveServo(const uint8_t position)
		{
			assert(m_pclDecoder);


			
			m_pclDecoder->m_clServo.write(position);
		}

		void Update()
		{
			if(m_uServoPos == m_pclDecoder->m_clServo.read())
		}

	private:
		ServoTurnoutDecoder *m_pclDecoder = nullptr;

		long				m_uTicks;
		uint8_t				m_uServoPos;
};

static int g_iTaskId = -1;
static uint8_t g_u8DecoderSlot = 255;
static ServoProgrammerHelper g_clHelper;

static void SendError_InvalidDecoderSlot(dcclite::Packet &packet, const uint32_t originalTaskId, const uint8_t slot)
{
	Session::detail::InitTaskPacket(packet, originalTaskId);

	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::FAILURE));
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgammerClientErrors::INVALID_DECODER_SLOT));
	packet.Write8(slot);

	Session::detail::SendTaskPacket(packet);
}

static void SendError_InvalidDecoderType(dcclite::Packet &packet, const uint32_t originalTaskId, dcclite::DecoderTypes type)
{
	Session::detail::InitTaskPacket(packet, originalTaskId);

	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::FAILURE));
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgammerClientErrors::INVALID_DECODER_TYPE));
	packet.Write8(static_cast<uint8_t>(type));

	Session::detail::SendTaskPacket(packet);
}


static void SendError_InvalidTaskId(dcclite::Packet &packet, const uint32_t originalTaskId)
{
	Session::detail::InitTaskPacket(packet, originalTaskId);

	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::FAILURE));
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgammerClientErrors::INVALID_TASK_ID));
	packet.Write32(g_iTaskId);

	Session::detail::SendTaskPacket(packet);
}

static void ParseStart(dcclite::Packet &packet, const uint32_t packetTaskId)
{	
	if (packetTaskId != g_iTaskId)
	{
		//
		//Another task started? Can do more than one per time...
		if (g_iTaskId >= 0)
		{
			SendError_InvalidTaskId(packet, packetTaskId);

			return;
		}

		//
		//a real start, so let do it...
		g_u8DecoderSlot = packet.ReadByte();
		auto decoder = DecoderManager::TryPopDecoder(g_u8DecoderSlot);
		if (decoder == nullptr)
		{
			SendError_InvalidDecoderSlot(packet, packetTaskId, g_u8DecoderSlot);

			return;
		}

		if (decoder->GetType() != dcclite::DecoderTypes::DEC_SERVO_TURNOUT)
		{
			//put it back
			DecoderManager::PushDecoder(decoder, g_u8DecoderSlot);

			SendError_InvalidDecoderType(packet, packetTaskId, decoder->GetType());

			return;
		}

		g_clHelper.SetDecoder(*static_cast<ServoTurnoutDecoder *>(decoder));		

		g_iTaskId = packetTaskId;			
	}

	//
	//else - already started, but packet may be lost, so answer server

	Session::detail::InitTaskPacket(packet, packetTaskId);
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::READY));

	Session::detail::SendTaskPacket(packet);
}

static void SendStopAck(dcclite::Packet &packet, const uint32_t taskId)
{
	Session::detail::InitTaskPacket(packet, taskId);
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::FINISHED));

	Session::detail::SendTaskPacket(packet);
}

static void ParseStop(dcclite::Packet &packet, const uint32_t packetTaskId)
{		
	if (g_iTaskId == packetTaskId)
	{
		SendStopAck(packet, packetTaskId);

		//
		// we must stop the programmer
		ServoProgrammer::Stop();
	}
	else if(g_iTaskId >= 0)
	{		
		//Wrong id... ignore...
		SendError_InvalidTaskId(packet, packetTaskId);
	}
	else
	{
		//
		// g_iTaskId < 0, so just report that stop was done...
		SendStopAck(packet, packetTaskId);
	}	
}

static void ParseMoveServo(dcclite::Packet &packet, const uint32_t packetTaskId)
{
	if (g_iTaskId != packetTaskId)
	{
		//Wrong id... ignore...
		SendError_InvalidTaskId(packet, packetTaskId);
	}

	const auto position = packet.ReadByte();

	Console::SendLogEx("[ParseMoveServo]", ' ', position);
	g_clHelper.MoveServo(position);

	Session::detail::InitTaskPacket(packet, packetTaskId);
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::SERVO_MOVED));
	packet.Write8(position);

	Session::detail::SendTaskPacket(packet);
}


void ServoProgrammer::Stop()
{
	if (g_iTaskId < 0)
		return;

	//If somehow programmer was started, but servo not acquired, check it first...
	if (g_clHelper.IsSet())
	{		
		auto *p = g_clHelper.ClearDecoder();			

		DecoderManager::PushDecoder(p, g_u8DecoderSlot);
	}

	g_iTaskId = -1;
}

void ServoProgrammer::ParsePacket(dcclite::Packet &packet)
{
	using namespace dcclite;

	const auto packetTaskId = packet.Read<uint32_t>();
	const auto serverMsg = static_cast<ServoProgrammerServerMsgTypes>(packet.ReadByte());

	switch (serverMsg)
	{
		case ServoProgrammerServerMsgTypes::START:
			ParseStart(packet, packetTaskId);
			break;

		case ServoProgrammerServerMsgTypes::STOP:
			ParseStop(packet, packetTaskId);
			break;

		case ServoProgrammerServerMsgTypes::MOVE_SERVO:
			ParseMoveServo(packet, packetTaskId);
			break;

		default:
			break;
	}
}