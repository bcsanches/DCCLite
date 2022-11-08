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

constexpr unsigned long DETACH_DELAY = 1000;
constexpr unsigned long DEGREE_DETACH_DELAY_BONUS = 50;

//20 seconds
constexpr unsigned long ZOMBIE_WAIT_TIME = 20000;

class ServoProgrammerHelper
{
	public:		
		ServoProgrammerHelper(const uint32_t taskId, const uint8_t slot, ServoTurnoutDecoder &decoder):					
			m_rclDecoder{ decoder },
			m_uTaskId{taskId},
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
			if (!m_rclDecoder.m_clServo.attached())			
			{
				//Console::SendLn("[ServoProgrammerHelper::MoveServo] attached");
				m_rclDecoder.m_clServo.attach(m_rclDecoder.m_clPin.Raw());
			}				

			auto bonus = abs(position - m_rclDecoder.m_clServo.read());

			m_rclDecoder.m_clServo.write(position);

			m_uDetachTime = millis() + DETACH_DELAY + (DEGREE_DETACH_DELAY_BONUS * bonus);
		}

		void UpdateServo(const uint8_t flags, const uint8_t startPos, const uint8_t endPos, const uint8_t ticks)
		{
			m_rclDecoder.m_fFlags = flags & ~dcclite::SRVT_STATE_BITS;
			m_rclDecoder.m_uStartPos = startPos;
			m_rclDecoder.m_uEndPos = endPos;
			m_rclDecoder.m_uTicks = ticks;			
		}

		void Update(const unsigned long ticks)
		{
			if ((ticks >= m_uDetachTime) && (m_rclDecoder.m_clServo.attached()))
			{
				m_rclDecoder.m_clServo.detach();

				//Console::SendLn("[ServoProgrammerHelper::MoveServo] detach");
			}
		}

		void EnableZombieMode(const unsigned long ticks)
		{
			m_uDetachTime = ticks + ZOMBIE_WAIT_TIME;
			m_fZombie = true;
		}

	private:
		ServoTurnoutDecoder &m_rclDecoder;

	public:
		ServoProgrammerHelper *m_pclNext = nullptr;
		ServoProgrammerHelper *m_pclPrev = nullptr;

		unsigned long		m_uDetachTime = 0;

		const uint32_t		m_uTaskId;		

		uint32_t			m_uServerSequence = 0;		

		const uint8_t		m_uDecoderSlot;

		bool				m_fZombie = false;
		
};

class TaskList
{
	public:
		void Insert(ServoProgrammerHelper *item) noexcept
		{
			item->m_pclNext = m_pclListHead;

			if (m_pclListHead)
			{
				m_pclListHead->m_pclPrev = item;
			}

			m_pclListHead = item;
		}

		void Remove(ServoProgrammerHelper *item) noexcept
		{
			if (item->m_pclPrev)
			{
				item->m_pclPrev->m_pclNext = item->m_pclNext;
			}

			if (item->m_pclNext)
			{
				item->m_pclNext->m_pclPrev = item->m_pclPrev;
			}

			if (m_pclListHead == item)
			{
				m_pclListHead = item->m_pclNext;
			}

			item->m_pclNext = item->m_pclPrev = nullptr;
		}

		ServoProgrammerHelper *TryFindTask(const uint32_t taskId) const noexcept
		{
			for (auto p = m_pclListHead; p; p = p->m_pclNext)
			{
				if (p->m_uTaskId == taskId)
					return p;
			}

			return nullptr;
		}

		void Clear() noexcept
		{
			while (m_pclListHead)
			{
				auto *next = m_pclListHead->m_pclNext;

				delete m_pclListHead;
				m_pclListHead = next;
			}
		}

		inline void Update(const unsigned long ticks)
		{
			for (auto item = m_pclListHead; item;)
			{
				if ((item->m_fZombie) && (item->m_uDetachTime <= ticks))
				{					
					this->Remove(item);

					auto next = item->m_pclNext;
					delete item;
					item = next;
				}
				else
				{
					item->Update(ticks);
					item = item->m_pclNext;
				}
					
			}
		}

	private:
		ServoProgrammerHelper *m_pclListHead = nullptr;
};

static TaskList g_clTasklist;

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
	auto task = g_clTasklist.TryFindTask(packetTaskId);

	if (task == nullptr)
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

		task = new ServoProgrammerHelper{ packetTaskId, slot, *static_cast<ServoTurnoutDecoder *>(decoder) };

		g_clTasklist.Insert(task);
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
	auto task = g_clTasklist.TryFindTask(packetTaskId);

	if(task == nullptr)	
	{
		//
		// g_iTaskId < 0, so just report that stop was done...
		SendStopAck(packet, packetTaskId);

		return;
	}
		
	SendStopAck(packet, packetTaskId);

	//
	// we must stop the programmer
	g_clTasklist.Remove(task);
	delete task;	
}

static void ParseMoveServo(dcclite::Packet &packet, const uint32_t packetTaskId)
{
	auto task = g_clTasklist.TryFindTask(packetTaskId);
	if (task == nullptr)
	{
		//Wrong id... ignore...
		SendError_InvalidTaskId(packet, packetTaskId, packetTaskId);

		return;
	}	

	const auto serverSequence = packet.Read<uint32_t>();

	//old packet?
	if (serverSequence < task->m_uServerSequence)
	{
		//drop it
		return;
	}		

	const auto position = packet.ReadByte();

	//Console::SendLogEx(F("[ParseMoveServo]"), ' ', position);
	DCCLITE_LOG << F("[ParseMoveServo] ") << position;

	task->MoveServo(position);

	task->m_uServerSequence = serverSequence;

	Session::detail::InitTaskPacket(packet, packetTaskId);
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::SERVO_MOVED));	
	packet.Write32(serverSequence);
	packet.Write8(position);

	Session::detail::SendTaskPacket(packet);
}

static void ParseDeployServo(dcclite::Packet &packet, const uint32_t packetTaskId)
{
	auto task = g_clTasklist.TryFindTask(packetTaskId);
	if (task == nullptr)
	{
		//Wrong id... ignore...
		SendError_InvalidTaskId(packet, packetTaskId, packetTaskId);

		return;
	}

	//Console::SendLogEx("[ParseDeployServo]", ' ', packetTaskId);


	//if task  is not a zombie, first deploy call, so do it
	if (!task->m_fZombie)
	{
		const auto flags = packet.ReadByte();
		const auto startPos = packet.ReadByte();
		const auto endPos = packet.ReadByte();
		const auto servoTicks = packet.ReadByte();

		task->UpdateServo(flags, startPos, endPos, servoTicks);
	}	

	Session::detail::InitTaskPacket(packet, packetTaskId);
	packet.Write8(static_cast<uint8_t>(dcclite::ServoProgrammerClientMsgTypes::DEPLOY_FINISHED));		
		
	Session::detail::SendTaskPacket(packet);

	/*
		Goto zombie mode

			In the perfect world, what happens is:
				Server -> sends DEPLOY_MSG
				client -> answers with DEPLOY_FINISHED and put task in zombie mode
				server -> sends a DEPLOY_FINISHED_ACK (and assumes that the task is gone)
				client -> when it receives DEPLOY_FINISHED_ACK, destroy the zombie task

			But, in the real world:
				- Server DEPLOY_MSG may never arrives, so server sends it again and again until 
					it gets a DEPLOY_FINISHED msg

				- Server may not receive a DEPLOY_FINISHED, so it keeps sending DEPLOY_MSG until it gets one

				- Server after getting a DEPLOY_FINISHED, sends a DEPLOY_FINISHED_ACK, but client may never see it,
					so the client timeouts the task after sending a DEPLOY_FINISHED
	
			After the ZOMBIE_TIMEOUT (20 seconds) the server could not ack, network is really sucking so 
			we probably will timeout before this happens, but just in case manage this and kill the task after 20 seconds

			We keep the task in zombie mode because the server may not see the first DEPLOY_FINISHED msg and will send MSG_DEPLOY again			
	*/
	task->EnableZombieMode(millis());
}

static void ParseDeployFinishedAck(dcclite::Packet &packet, const uint32_t packetTaskId)
{
	auto task = g_clTasklist.TryFindTask(packetTaskId);
	if (task == nullptr)
	{
		//Wrong id... ignore...
		SendError_InvalidTaskId(packet, packetTaskId, packetTaskId);

		return;
	}

	//Console::SendLogEx("[ParseDeployServo]", ' ', packetTaskId);


	//if task  is not a zombie, first deploy call, so do it
	if (!task->m_fZombie)
	{
		//
		//Wtf? The task should be in zombie mode ... ignore

		//Console::SendLogEx(F("ParseDeployFinishedAck"), F("Expected task in Zombie mode"), ' ', packetTaskId);
		DCCLITE_LOG << F("[ParseDeployFinishedAck] Expected task in Zombie mode ") << packetTaskId << DCCLITE_ENDL;

		return;
	}

	g_clTasklist.Remove(task);
	delete task;
}


void ServoProgrammer::Stop()
{
	g_clTasklist.Clear();	
}

void ServoProgrammer::Update(const unsigned long ticks)
{
	g_clTasklist.Update(ticks);
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

		case ServoProgrammerServerMsgTypes::DEPLOY:
			ParseDeployServo(packet, packetTaskId);
			break;

		case ServoProgrammerServerMsgTypes::DEPLOY_FINISHED_ACK:
			ParseDeployFinishedAck(packet, packetTaskId);
			break;

		default:
			break;
	}
}
