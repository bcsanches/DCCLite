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

#include "Packet.h"
#include "Session.h"

static int g_iTaskId = -1;


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
		
		g_iTaskId = packetTaskId;

		//
		//a real start, so let do it...
		//TODO
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
	if (g_iTaskId != packetTaskId)
	{
		if (g_iTaskId >= 0)
		{
			//Wrong id... ignore...
			SendError_InvalidTaskId(packet, packetTaskId);

			return;
		}					

		//
		// g_iTaskId < 0, so just report that stop was done...
		SendStopAck(packet, packetTaskId);

		return;
	}

	//
	// we must stop the programmer
	ServoProgrammer::Stop();
}


void ServoProgrammer::Stop()
{
	if (g_iTaskId < 0)
		return;

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

		default:
			break;
	}
}