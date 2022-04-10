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
		ServoProgrammerHelper(const uint32_t taskId, const uint8_t slot, ServoTurnoutDecoder &decoder):
			m_uTaskId{taskId},			
			m_rclDecoder{ decoder },
			m_uDecoderSlot{ slot }			
		{
			m_rclDecoder.PM_EnableProgMode();
		}		

		~ServoProgrammerHelper()
		{
			m_rclDecoder.PM_DisableProgMode();

			DecoderManager::PushDecoder(&m_rclDecoder, m_uDecoderSlot);
		}

		void MoveServo(const uint8_t position)
		{						
			m_rclDecoder.m_clServo.write(position);
		}

	private:
		ServoTurnoutDecoder &m_rclDecoder;

	public:
		const unsigned int	m_uTaskId;
		const uint8_t		m_uDecoderSlot;		

		uint32_t			m_uServerSequence = 0;		
};

static ServoProgrammerHelper *g_pclHelper = nullptr;

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


static void SendError_InvalidTaskId(dcclite::Packet &packet, const uint32_t originalTaskId, const uint32_t currentTaskId)
{
	Session::detail::InitTaskPacket(packet, originalTaskId);

	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::FAILURE));
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgammerClientErrors::INVALID_TASK_ID));
	packet.Write32(currentTaskId);

	Session::detail::SendTaskPacket(packet);
}

static void ParseStart(dcclite::Packet &packet, const uint32_t packetTaskId)
{	
	if (g_pclHelper == nullptr)
	{
		//
		//a real start, so let do it...
		auto slot = packet.ReadByte();
		auto decoder = DecoderManager::TryPopDecoder(slot);
		if (decoder == nullptr)
		{
			SendError_InvalidDecoderSlot(packet, packetTaskId, slot);

			return;
		}

		if (decoder->GetType() != dcclite::DecoderTypes::DEC_SERVO_TURNOUT)
		{
			//put it back
			DecoderManager::PushDecoder(decoder, slot);

			SendError_InvalidDecoderType(packet, packetTaskId, decoder->GetType());

			return;
		}

		g_pclHelper = new ServoProgrammerHelper{ packetTaskId, slot, *static_cast<ServoTurnoutDecoder *>(decoder) };		
	}	
	else if (g_pclHelper->m_uTaskId != packetTaskId)
	{
		//
		//Another task started? Can do more than one per time...
		SendError_InvalidTaskId(packet, packetTaskId, g_pclHelper->m_uTaskId);

		return;
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
	if (g_pclHelper == nullptr)
	{
		//
		// g_iTaskId < 0, so just report that stop was done...
		SendStopAck(packet, packetTaskId);

		return;
	}
	
	if (g_pclHelper->m_uTaskId == packetTaskId)
	{		
		SendStopAck(packet, packetTaskId);

		//
		// we must stop the programmer
		ServoProgrammer::Stop();
	}
	else
	{
		//Wrong id... ignore...
		SendError_InvalidTaskId(packet, packetTaskId, g_pclHelper->m_uTaskId);
	}	
}

static void ParseMoveServo(dcclite::Packet &packet, const uint32_t packetTaskId)
{
	if((!g_pclHelper) || (g_pclHelper->m_uTaskId != packetTaskId))	
	{
		//Wrong id... ignore...
		SendError_InvalidTaskId(packet, packetTaskId, g_pclHelper->m_uTaskId);
	}

	const auto serverSequence = packet.Read<uint32_t>();

	//old packet?
	if (serverSequence < g_pclHelper->m_uServerSequence)
	{
		//drop it
		return;
	}		

	const auto position = packet.ReadByte();

	Console::SendLogEx("[ParseMoveServo]", ' ', position);
	g_pclHelper->MoveServo(position);

	g_pclHelper->m_uServerSequence = serverSequence;

	Session::detail::InitTaskPacket(packet, packetTaskId);
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::SERVO_MOVED));	
	packet.Write32(serverSequence);
	packet.Write8(position);

	Session::detail::SendTaskPacket(packet);
}


void ServoProgrammer::Stop()
{
	if (g_pclHelper == nullptr)
		return;

	delete g_pclHelper;
	g_pclHelper = nullptr;	
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