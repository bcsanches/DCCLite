#include "LoconetService.h"

#include <Log.h>

#include <exception>
#include <optional>
#include <deque>

#include "magic_enum.hpp"

#include "Broker.h"
#include "Clock.h"
#include "ThrottleService.h"
#include "SerialPort.h"

enum Bits : uint8_t
{
	BIT_0 = 0x01,	
	BIT_1 = 0x02,
	BIT_2 = 0x04,
	BIT_3 = 0x08,
	BIT_4 = 0x10,
	BIT_5 = 0x20,
	BIT_6 = 0x40,
	BIT_7 = 0x80	
};


//based on https://www.digitrax.com/static/apps/cms/media/documents/loconet/loconetpersonaledition.pdf
enum Opcodes : uint8_t
{
	OPC_ERROR_MOVE_SLOTS = 0x3A,
	OPC_ERROR_LOCO_ADR = 0x3F,

	OPC_LOCO_SPD = 0xA0,	
	OPC_LOCO_DIRF = 0xA1,
	OPC_LOCO_SND = 0xA2,
	OPC_LONG_ACK = 0xB4,
	OPC_SLOT_STAT1 = 0xB5,
	OPC_MOVE_SLOTS = 0xBA,
	OPC_RQ_SL_DATA = 0xBB,
	OPC_LOCO_ADR = 0xBF,

	//See JMRI - PR3Adapter.java configure()
	OPC_UNDOC_SETMS100 = 0xD3,
	OPC_PANEL_RESPONSE = 0xD7,
	OPC_UNDOC_PANEL_QUERY = 0xDF,

	OPC_SL_RD_DATA = 0xE7,
	OPC_IMM_PACKET = 0xED,
	OPC_WR_SL_DATA = 0xEF
};

using namespace std::chrono_literals;

static auto constexpr PURGE_INTERVAL = 100s;
static auto constexpr PURGE_TIMEOUT = 200s;

static constexpr auto MAX_LN_MESSAGE_LEN = 20;
static constexpr auto MAX_SLOTS = 120;

static constexpr auto SLOT_STAT_SPEED_STEPS_BITS = (BIT_0 | BIT_1);	//011 = send 128 speed mode packets


/**
D7 - 0; always 0
D6 - SL_XCNT; reserved, set 0
D5 - SL_DIR; 1 = loco direction FORWARD
D4 - SL_F0; 1 = Directional lighting ON
D3 - SL_F4; 1 = F4 ON
D2 - SL_F3; 1 = F3 ON
D1 - SL_F2; 1 = F2 ON
D0 - SL_F1; 1 = F1 ON
*/
static constexpr auto SLOT_SPEED_DIR_BIT = BIT_5;
static constexpr auto SLOT_SPEED_F0 = BIT_4;
static constexpr auto SLOT_SPEED_F4 = BIT_3;
static constexpr auto SLOT_SPEED_F3 = BIT_2;
static constexpr auto SLOT_SPEED_F2 = BIT_1;
static constexpr auto SLOT_SPEED_F1 = BIT_0;

static constexpr auto MAX_SLOT_FUNCTIONS = 32;

typedef dcclite::BasePacket<MAX_LN_MESSAGE_LEN> MiniPacket_t;

static dcclite::broker::ThrottleService *g_pclThrottleService = nullptr;

uint8_t DefaultMsgSizes(const Opcodes opcode)
{
	switch(opcode)
	{
		case OPC_LONG_ACK:
			return 4;			

		case OPC_SL_RD_DATA:
			return 0x0E;

		case OPC_WR_SL_DATA:
			return 14;

		case OPC_UNDOC_SETMS100:
			return 6;

		default:
			throw std::exception(fmt::format("[LoconetService::DefaultMsgSizes] Unknown opcode: {}", opcode).c_str());
	}
}

class LoconetMessageWriter
{
	public:
		LoconetMessageWriter(Opcodes opcode)
		{
			m_tPacket.Write8(opcode);
			
			m_uMsgLen = DefaultMsgSizes(opcode);
			if (m_uMsgLen > 6)
			{
				m_tPacket.Write8(m_uMsgLen);				
			}
		}

		void WriteByte(const uint8_t byte)
		{
			if (m_tPacket.GetSize() >= m_uMsgLen - 1)
				throw std::exception(fmt::format("[LoconetService::WriteByte] Buffer overflow").c_str());

			m_tPacket.Write8(byte & 0x7F);			
		}

		const uint8_t *PackMsg()
		{
			uint8_t checksum = 0xFF;

			m_tPacket.Seek(0);
			for (int i = 0; i < m_uMsgLen - 1; ++i)
			{
				checksum ^= m_tPacket.ReadByte();
			}

			m_tPacket.Write8(checksum);

			return m_tPacket.GetData();
		}

		uint8_t GetMsgLen() const
		{
			return m_uMsgLen;
		}

	private:
		MiniPacket_t m_tPacket;

		uint8_t m_uMsgLen = 2;
};

///////////////////////////////////////////////////////////////////////////////
//
// Loconet Slot
//
///////////////////////////////////////////////////////////////////////////////




class Slot: public dcclite::broker::ILoconetSlot
{
	public:
		enum class States
		{
			//Those values match the bitmask used by loconet on bits D4 and D5
			FREE = 0x00,
			COMMON = BIT_4,
			IDLE = BIT_5,
			IN_USE = BIT_4 | BIT_5
		};	

		Slot()
		{
			m_arFunctions.ClearAll();
		}

		~Slot()
		{
			this->ReleaseThrottle();
		}		

		void SetId(const uint8_t id) noexcept 
		{
			m_uId = id;
		}

		//
		//
		// STATE management
		//
		//

		bool IsFree() const noexcept
		{
			return m_eState == States::FREE;
		}	

		bool IsInUse() const noexcept
		{
			return m_eState == States::IN_USE;
		}

		void GotoState_Common() noexcept
		{
			dcclite::Log::Debug("[Slot[{}]::GotoState_Common] from {}", this->GetId(),  magic_enum::enum_name(m_eState));

			m_eState = States::COMMON;		

			this->ReleaseThrottle();
		}

		void GotoState_Common(const dcclite::broker::DccAddress addr) noexcept
		{
			dcclite::Log::Debug("[Slot[{}]::GotoState_Common] from {}, new address {}", this->GetId(), magic_enum::enum_name(m_eState), addr);

			this->GotoState_Common();
			
			m_tLocomotiveAddress = addr;
		}

		//HACK for slot 0
		void GotoState_Reserved() noexcept
		{
			m_eState = States::IN_USE;
		}

		void GotoState_InUse() noexcept
		{
			dcclite::Log::Debug("[Slot[{}]::GotoState_InUse] from {}", this->GetId(), magic_enum::enum_name(m_eState));

			m_eState = States::IN_USE;

			/*
				sometimes a loconet throttle may simple ask to re - use a slot, so do not re - create a network throttle.

				This happens when the throttle simple ask a null move before freeing a slot (setting it to common state)

				This can be forced rapid clicking on the "loco" button on a loconet throttle, sometimes it fires a "null move" 
				message before firing a "setStat1" that will set a slot to common state
			*/
			if(!m_pclThrottle)
				m_pclThrottle = &g_pclThrottleService->CreateThrottle(*this);			
		}

		void GotoState_Free() noexcept
		{
			dcclite::Log::Debug("[Slot[{}]::GotoState_Free] from {}", this->GetId(), magic_enum::enum_name(m_eState));

			m_eState = States::FREE;

			this->ReleaseThrottle();
		}

		States GetState() const noexcept
		{
			return m_eState;
		}

		//
		//
		// Locomotive state controls
		//
		//		


		void SetForward(bool v) noexcept
		{
			m_fForward = v;

			if (m_pclThrottle)
				m_pclThrottle->OnForwardChange();
		}				

		void SetFunctions(const bool *beginFunction, const bool *endFunction, const uint8_t beginIndex) noexcept
		{
			assert((endFunction - beginFunction) <= MAX_SLOT_FUNCTIONS);
			
			auto index = beginIndex;
			for (; beginFunction != endFunction; ++beginFunction, ++index)
				m_arFunctions.SetBitValue(index, *beginFunction);

			if (m_pclThrottle)
				m_pclThrottle->OnFunctionChange(beginIndex, index);
		}		

		void SetSpeed(const uint8_t speed) noexcept
		{
			m_uSpeed = speed;

			if (m_pclThrottle)
				m_pclThrottle->OnSpeedChange();
		}		

		void EmergencyStop() noexcept
		{
			m_uSpeed = 0;

			if (m_pclThrottle)
				m_pclThrottle->OnEmergencyStop();
		}

	private:
		void ReleaseThrottle()
		{
			if ((!g_pclThrottleService) || (!m_pclThrottle))
				return;

			g_pclThrottleService->ReleaseThrottle(*m_pclThrottle);
			m_pclThrottle = nullptr;
		}

	private:		
		dcclite::broker::IThrottle *m_pclThrottle = nullptr;		

		States m_eState = States::FREE;		
};

///////////////////////////////////////////////////////////////////////////////
//
// Helpers
//
///////////////////////////////////////////////////////////////////////////////

//    D5 D4
// 00 1  1    0000
static constexpr auto SLOT_STAT_USAGE_MASK = 0x30;

static uint8_t BuildSlotStatByte(const Slot &slot) noexcept
{
	//;011=send 128 speed mode packets
	return  BIT_0 | BIT_1 | static_cast<uint8_t>(slot.GetState());
}

static uint8_t BuildSlotDirfByte(const Slot &slot) noexcept
{
	auto functions = slot.GetFunctions();

	//D7 - 0; always 0
	//D6 - SL_XCNT; reserved, set 0
	//D5 - SL_DIR; 1 = loco direction FORWARD
	//D4 - SL_F0; 1 = Directional lighting ON
	//D3 - SL_F4; 1 = F4 ON
	//D2 - SL_F3; 1 = F3 ON
	//D1 - SL_F2; 1 = F2 ON
	//D0 - SL_F1; 1 = F1 ON
	return
		(slot.IsForwardDir() ? 0 : BIT_5) |
		(functions[0] ? BIT_4 : 0) |
		(functions[4] ? BIT_3 : 0) |
		(functions[3] ? BIT_2 : 0) |
		(functions[2] ? BIT_1 : 0) |
		(functions[1] ? BIT_0 : 0);
}

static Slot::States ParseStatByte(const uint8_t stat) noexcept
{
	return static_cast<Slot::States>(stat & SLOT_STAT_USAGE_MASK);
}

///////////////////////////////////////////////////////////////////////////////
//
// SlotManager - Yes! A Manager!
//
///////////////////////////////////////////////////////////////////////////////

class SlotManager
{
	public:
		SlotManager();

		std::optional<uint8_t> AcquireLocomotive(const dcclite::broker::DccAddress address, const dcclite::Clock::TimePoint_t ticks);

		void SetSlotToInUse(uint8_t slot, const dcclite::Clock::TimePoint_t ticks);
		void SetSlotFree(uint8_t slot);

		void SetLocomotiveSpeed(const uint8_t slot, const uint8_t speed, const dcclite::Clock::TimePoint_t ticks) noexcept;

		void EmergencyStop(const uint8_t slot, const dcclite::Clock::TimePoint_t ticks) noexcept;

		void SetForward(const uint8_t slot, const bool forward, const dcclite::Clock::TimePoint_t ticks);
		void SetFunctions(const uint8_t slot, const bool *beginFunction, const bool *endFunction, uint8_t beginIndex, const dcclite::Clock::TimePoint_t ticks);

		LoconetMessageWriter MakeMessage_SlotReadData(const uint8_t slot) const;

		void ForceSlotState(const uint8_t slot, const Slot::States state, const dcclite::Clock::TimePoint_t ticks);

		void Serialize(dcclite::JsonOutputStream_t &stream) const;
		inline void SerializeSlot(const uint8_t slotIndex, dcclite::JsonOutputStream_t &stream) const;

		void PurgeSlots(const dcclite::Clock::TimePoint_t ticks, std::function<void (uint8_t)> callback) noexcept;

	private:
		const Slot &GetSlot(const uint8_t slot) const;
		Slot *TryGetLocomotiveSlot(const uint8_t slot);		

		void SerializeSlot(const Slot &slot, dcclite::JsonOutputStream_t &stream) const;

		void RefreshSlotTimeout(const uint8_t slot, const dcclite::Clock::TimePoint_t ticks) noexcept;

	private:
		std::array<Slot, MAX_SLOTS> m_arSlots;

		std::array<dcclite::Clock::TimePoint_t, MAX_SLOTS> m_arSlotsTimeout;
};

SlotManager::SlotManager()
{	
	//dispatch slot - never use
	m_arSlots[0].GotoState_Reserved();

	for (int i = 0; i < MAX_SLOTS; ++i)
		m_arSlots[i].SetId(i);
}

std::optional<uint8_t> SlotManager::AcquireLocomotive(const dcclite::broker::DccAddress address, const dcclite::Clock::TimePoint_t ticks)
{
	auto it = std::find_if(m_arSlots.begin(), m_arSlots.end(), [address](Slot &slot)
		{			
			return !slot.IsFree() && (slot.GetLocomotiveAddress() == address);
		}
	);

	//address not found?
	if (it == m_arSlots.end())
	{		
		//look for a free slot
		it = std::find_if(m_arSlots.begin(), m_arSlots.end(), [](Slot &slot) {return slot.IsFree(); });
		if (it == m_arSlots.end())
			return false;

		//Found it, use...
		it->GotoState_Common(address);		
	}

	auto index = it - m_arSlots.begin();	
	
	return index;
}

const Slot &SlotManager::GetSlot(const uint8_t slot) const
{	
	return m_arSlots.at(slot);
}

Slot *SlotManager::TryGetLocomotiveSlot(const uint8_t slot)
{
	if (slot == 0)
	{
		dcclite::Log::Error("[SlotManager::TryGetLocomotiveSlot] Slot 0 is not a locomotive, it is a dispatcher slot");

		return nullptr;
	}

	if (slot >= MAX_SLOTS)
	{
		dcclite::Log::Error("[SlotManager::TryGetLocomotiveSlot] Slot {} is outside locomotive range, trying to access special slot?", slot);

		return nullptr;
	}

	return &m_arSlots[slot];
}


void SlotManager::SetSlotToInUse(uint8_t slot, const dcclite::Clock::TimePoint_t ticks)
{
	m_arSlots.at(slot).GotoState_InUse();
	this->RefreshSlotTimeout(slot, ticks);
}

void SlotManager::SetSlotFree(uint8_t slot)
{
	m_arSlots.at(slot).GotoState_Free();
}

LoconetMessageWriter SlotManager::MakeMessage_SlotReadData(const uint8_t slotIndex) const
{
	auto &slot = this->GetSlot(slotIndex);

	//<0xE7>,<0E>,<SLOT#>,<STAT>,<ADR>,<SPD>,<DIRF>,<TRK> <SS2>, <ADR2>, <SND>, <ID1>, <ID2>, <CHK>
	LoconetMessageWriter msg(OPC_SL_RD_DATA);

	const auto rawLocoAddress = slot.GetLocomotiveAddress().GetAddress();

	msg.WriteByte(slotIndex);
	msg.WriteByte(BuildSlotStatByte(slot));			//STAT
	msg.WriteByte(rawLocoAddress & 0x7F);			//ADDR
	msg.WriteByte(slot.GetSpeed() & 0x7F);			//SPD
	msg.WriteByte(BuildSlotDirfByte(slot));			//DIRF
	msg.WriteByte(0x01);							//TRK
	msg.WriteByte(0);								//SS2
	msg.WriteByte((rawLocoAddress >> 7) & 0x7F);	//ADDR2
	msg.WriteByte(0);								//SND
	msg.WriteByte(0);								//Id1
	msg.WriteByte(0);								//Id2

	return msg;
}

void SlotManager::SetLocomotiveSpeed(const uint8_t slot, const uint8_t speed, const dcclite::Clock::TimePoint_t ticks) noexcept
{
	auto pSlot = this->TryGetLocomotiveSlot(slot);

	if (!pSlot)
		return;

	pSlot->SetSpeed(speed);	
	this->RefreshSlotTimeout(slot, ticks);
	
}

void SlotManager::EmergencyStop(const uint8_t slot, const dcclite::Clock::TimePoint_t ticks) noexcept
{
	auto pSlot = this->TryGetLocomotiveSlot(slot);

	if (!pSlot)
		return;

	pSlot->EmergencyStop();
	this->RefreshSlotTimeout(slot, ticks);
}

void SlotManager::SetForward(const uint8_t slot, const bool forward, const dcclite::Clock::TimePoint_t ticks)
{
	auto pSlot = this->TryGetLocomotiveSlot(slot);

	if (!pSlot)
		return;

	pSlot->SetForward(forward);
	this->RefreshSlotTimeout(slot, ticks);
}

void SlotManager::SetFunctions(const uint8_t slot, const bool *beginFunction, const bool *endFunction, uint8_t beginIndex, const dcclite::Clock::TimePoint_t ticks)
{
	auto pSlot = this->TryGetLocomotiveSlot(slot);

	if (!pSlot)
		return;

	pSlot->SetFunctions(beginFunction, endFunction, beginIndex);
	this->RefreshSlotTimeout(slot, ticks);
}

void SlotManager::ForceSlotState(const uint8_t slot, const Slot::States state, const dcclite::Clock::TimePoint_t ticks)
{
	auto &slotHandle = m_arSlots.at(slot);

	switch(state)
	{
		case Slot::States::COMMON:
			slotHandle.GotoState_Common();
			break;

		case Slot::States::FREE:
			slotHandle.GotoState_Free();
			break;

		case Slot::States::IN_USE:
			slotHandle.GotoState_InUse();
			break;

		default:
			dcclite::Log::Error("[SlotManager::ForceSlotState] Force slot {} state to {} not supported.", slot, state);
			break;
	}	

	this->RefreshSlotTimeout(slot, ticks);
}

void SlotManager::RefreshSlotTimeout(const uint8_t slot, const dcclite::Clock::TimePoint_t ticks) noexcept
{
	m_arSlotsTimeout[slot] = ticks + PURGE_TIMEOUT;
}

void SlotManager::SerializeSlot(const uint8_t slotIndex, dcclite::JsonOutputStream_t &stream) const
{
	assert(slotIndex < m_arSlots.size());	

	this->SerializeSlot(m_arSlots[slotIndex], stream);
}

void SlotManager::SerializeSlot(const Slot &slot, dcclite::JsonOutputStream_t &slotData) const
{
	slotData.AddStringValue("state", magic_enum::enum_name(slot.GetState()));
	slotData.AddIntValue("speed", slot.GetSpeed());
	slotData.AddIntValue("locomotiveAddress", slot.GetLocomotiveAddress().GetAddress());
	slotData.AddBool("forward", slot.IsForwardDir());	

	auto f = slot.GetFunctions();

	int32_t functionsData;
	memcpy(&functionsData, f.GetRaw(), sizeof(functionsData));

	slotData.AddIntValue("functions", functionsData);
}

void SlotManager::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	auto slotsData = stream.AddArray("slots");

	for (auto &slot : m_arSlots)
	{
		auto slotData = slotsData.AddObject();

		this->SerializeSlot(slot, slotData);		
	}
}

void SlotManager::PurgeSlots(const dcclite::Clock::TimePoint_t ticks, std::function<void(uint8_t)> callback) noexcept
{	
	for(unsigned i = 1; i < m_arSlots.size(); ++i)	
	{
		if (m_arSlots[i].IsInUse() && (m_arSlotsTimeout[i] <= ticks))
		{
			m_arSlots[i].GotoState_Common();

			callback(i);
		}
	}

}


namespace dcclite::broker
{

	///////////////////////////////////////////////////////////////////////////////
	//
	// MessageDispatcher
	//
	///////////////////////////////////////////////////////////////////////////////


	class MessageDispatcher
	{
		public:
			void Send(const LoconetMessageWriter &msg, SerialPort &port);

			void Update(SerialPort &port);

		private:
			void SendData(const uint8_t *data, unsigned size, SerialPort &port);

			void PumpMessages(SerialPort &port);

		private:
			SerialPort::DataPacket m_clOutputPacket;

			std::deque<LoconetMessageWriter> m_clOutputQueue;
	};

	void MessageDispatcher::Send(const LoconetMessageWriter &msg, SerialPort &port)
	{
		//is queue empty and packet idle?
		if (m_clOutputQueue.empty() && m_clOutputPacket.IsDataReady())
		{
			//yes, so dispatch it imediatelly
			LoconetMessageWriter localMsg{ msg };

			this->SendData(localMsg.PackMsg(), localMsg.GetMsgLen(), port);
		}
		else
		{
			m_clOutputQueue.push_back(msg);

			//The OutputPacket may be idle now, so try to pump
			this->PumpMessages(port);
		}
	}

	void MessageDispatcher::PumpMessages(SerialPort &port)
	{
		if (!m_clOutputPacket.IsDataReady())
			return;

		if (m_clOutputQueue.empty())
			return;		

		auto msg = m_clOutputQueue.front();
		m_clOutputQueue.pop_front();

		this->SendData(msg.PackMsg(), msg.GetMsgLen(), port);
	}

	void MessageDispatcher::Update(SerialPort &port)
	{
		this->PumpMessages(port);
	}

	void MessageDispatcher::SendData(const uint8_t *data, unsigned int size, SerialPort &port)
	{
		if (!m_clOutputPacket.IsDataReady())
			throw std::logic_error("[MessageDispatcher::SendData] Do not call me when m_clOutputPacket is busy");

		m_clOutputPacket.Clear();
		m_clOutputPacket.WriteData(data, size);

		port.Write(m_clOutputPacket);
	}

	///////////////////////////////////////////////////////////////////////////////
	//
	// LoconetServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	class LoconetServiceImpl : public LoconetService
	{
		public:
			LoconetServiceImpl(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);
			~LoconetServiceImpl() override;

			void Update(const dcclite::Clock &clock) override;

			void Initialize() override;

			static std::unique_ptr<Service> Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

			void Serialize(JsonOutputStream_t &stream) const override;

		private:
			void DispatchLnMessage(LoconetMessageWriter &msg);
			void DispatchLnLongAckMessage(const Opcodes opcode, const uint8_t responseCode);

			void ParseMessage(const uint8_t opcode, MiniPacket_t &payload, const dcclite::Clock::TimePoint_t ticks);

			void NotifySlotChanged(uint8_t slotIndex) const;

			void ParseLocomotiveDirf(const uint8_t slot, const uint8_t dirf, const dcclite::Clock::TimePoint_t ticks);
			void ParseLocomotiveSnd(const uint8_t slot, const uint8_t snd, const dcclite::Clock::TimePoint_t ticks);

		private:
			SlotManager m_clSlotManager;

			SerialPort  m_clSerialPort;		

			SerialPort::DataPacket m_clInputPacket;			

			MessageDispatcher m_clMessageDispatcher;

			std::string m_strThrottleServiceName;

			dcclite::Clock::TimePoint_t m_tNextPurgeTicks;	
	};


	LoconetServiceImpl::LoconetServiceImpl(const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
		LoconetService(name, broker, params, project),
		m_clSerialPort(params["port"].GetString()),
		m_strThrottleServiceName(params["throttleService"].GetString())
	{				
		dcclite::Log::Info("[LoconetService] Started, listening on port {}", params["port"].GetString());

		LoconetMessageWriter msg(OPC_UNDOC_SETMS100);

		msg.WriteByte(0x10);
		msg.WriteByte(3);
		msg.WriteByte(0);
		msg.WriteByte(0);

		this->DispatchLnMessage(msg);

		m_clSerialPort.Read(m_clInputPacket);
	}
	

	LoconetServiceImpl::~LoconetServiceImpl()
	{
		//empty
	}

	void LoconetServiceImpl::Initialize()
	{
		g_pclThrottleService = static_cast<ThrottleService *>(m_rclBroker.TryFindService(m_strThrottleServiceName));

		if (!g_pclThrottleService)
			throw std::runtime_error(fmt::format("[LoconetServiceImpl::Initialize] Cannot find throttle service instance: {}", m_strThrottleServiceName));
	}	

	void LoconetServiceImpl::DispatchLnLongAckMessage(const Opcodes opcode, const uint8_t responseCode)
	{
		LoconetMessageWriter msg(OPC_LONG_ACK);

		msg.WriteByte(opcode);
		msg.WriteByte(responseCode);

		this->DispatchLnMessage(msg);
	}

	void LoconetServiceImpl::DispatchLnMessage(LoconetMessageWriter &msg)
	{
		m_clMessageDispatcher.Send(msg, m_clSerialPort);		
	}

	void LoconetServiceImpl::ParseLocomotiveDirf(const uint8_t slot, const uint8_t dirf, const dcclite::Clock::TimePoint_t ticks)
	{
		bool forward = (dirf & BIT_5) == 0;

		bool functions[5];
		functions[0] = dirf & BIT_4;
		functions[1] = dirf & BIT_0;
		functions[2] = dirf & BIT_1;
		functions[3] = dirf & BIT_2;
		functions[4] = dirf & BIT_3;

		m_clSlotManager.SetForward(slot, forward, ticks);
		m_clSlotManager.SetFunctions(slot, functions, functions + 5, 0, ticks);

		this->NotifySlotChanged(slot);
	}

	void LoconetServiceImpl::ParseLocomotiveSnd(const uint8_t slot, const uint8_t snd, const dcclite::Clock::TimePoint_t ticks)
	{
		bool functions[4];

		//F5
		functions[0] = snd & BIT_0;

		//F6
		functions[1] = snd & BIT_1;

		//F7
		functions[2] = snd & BIT_2;

		//F8
		functions[3] = snd & BIT_3;

		m_clSlotManager.SetFunctions(slot, functions, functions + 4, 5, ticks);
		this->NotifySlotChanged(slot);
	}

	void LoconetServiceImpl::ParseMessage(const uint8_t opcode, MiniPacket_t &payload, const dcclite::Clock::TimePoint_t ticks)
	{
		switch (opcode)
		{
			case Opcodes::OPC_IMM_PACKET:
				{
					payload.ReadByte();	//0x0D
					payload.ReadByte();	//0x7F


				}
				break;

			case Opcodes::OPC_LOCO_DIRF:
				{
					uint8_t slot = payload.ReadByte();
					uint8_t dirf = payload.ReadByte();

					Log::Trace("[LoconetServiceImpl::Update] Setting DIRF {} for slot {}", dirf, slot);
					this->ParseLocomotiveDirf(slot, dirf, ticks);
				}
				break;

			case Opcodes::OPC_LOCO_SND:
				{
					uint8_t slot = payload.ReadByte();
					uint8_t snd = payload.ReadByte();

					Log::Trace("[LoconetServiceImpl::Update] Setting SND {} for slot {}", snd, slot);
					this->ParseLocomotiveSnd(slot, snd, ticks);
				}
				break;

			case Opcodes::OPC_LOCO_SPD:
				{
					uint8_t slot = payload.ReadByte();					
					uint8_t speed = payload.ReadByte();					

					//Loconet speed 1 means "emergency stop", so handle it
					if (speed == 1)
					{
						Log::Trace("[LoconetServiceImpl::Update] Emergency stop for slot {}", slot);

						m_clSlotManager.EmergencyStop(slot, ticks);
					}
					else
					{
						Log::Trace("[LoconetServiceImpl::Update] Setting speed {} for slot {}", speed, slot);
						m_clSlotManager.SetLocomotiveSpeed(slot, speed, ticks);
					}
					

					this->NotifySlotChanged(slot);
				}
				break;

			case Opcodes::OPC_MOVE_SLOTS:
				{
					uint8_t src = payload.ReadByte();					
					uint8_t dest = payload.ReadByte();					

					//is a null move)
					if (src == dest)
					{
						Log::Trace("[LoconetServiceImpl::ParseMessage] MoveSlots: null move {} started", src);

						m_clSlotManager.SetSlotToInUse(src, ticks);

						auto msg = m_clSlotManager.MakeMessage_SlotReadData(src);

						this->DispatchLnMessage(msg);

						Log::Trace("[LoconetServiceImpl::ParseMessage] MoveSlots: null move {} completed", src);
						this->NotifySlotChanged(src);
					}
					else if (dest == 0)
					{
						m_clSlotManager.SetSlotFree(src);
						Log::Trace("[LoconetServiceImpl::ParseMessage] MoveSlots: dispached slot {}", src);

						this->NotifySlotChanged(src);
					}
					else if (src == 0)
					{
						Log::Error("[LoconetServiceImpl::ParseMessage] MoveSlots: Dispatch GET not supported", src);

						DispatchLnLongAckMessage(Opcodes::OPC_MOVE_SLOTS, 0);
					}
					else
					{
						Log::Error("[LoconetServiceImpl::ParseMessage] MoveSlots: movement not supported, src: {}, dest: {}", src, dest);					
						this->DispatchLnLongAckMessage(OPC_ERROR_MOVE_SLOTS, 0);
					}
				}
				break;

			case Opcodes::OPC_RQ_SL_DATA:
				{
					uint8_t slot = payload.ReadByte();

					if (slot >= MAX_SLOTS)
					{
						Log::Error("[LoconetServiceImpl::ParseMessage] OPC_RQ_SL_DATA: requesting for invalid slot {}", slot);

						DispatchLnLongAckMessage(Opcodes::OPC_RQ_SL_DATA, 0);
						break;
					}
					auto response = m_clSlotManager.MakeMessage_SlotReadData(slot);

					this->DispatchLnMessage(response);

					Log::Trace("[LoconetServiceImpl::Update] Request slot {} data", slot);
				}
				break;

			case OPC_SLOT_STAT1:
				{
					const uint8_t slot = payload.ReadByte();					

					if (slot >= MAX_SLOTS)
					{
						Log::Error("[LoconetServiceImpl::ParseMessage] OPC_SLOT_STAT1: requesting for invalid slot {}", slot);

						DispatchLnLongAckMessage(Opcodes::OPC_RQ_SL_DATA, 0);
						break;
					}

					const uint8_t stat = payload.ReadByte();
					Log::Trace("[LoconetServiceImpl::Update] Write slot {} stat1 {:#b}", slot, stat);

					m_clSlotManager.ForceSlotState(slot, ParseStatByte(stat), ticks);
					this->NotifySlotChanged(slot);
				}
				break;

			case Opcodes::OPC_LOCO_ADR:
				{
					//<0xBF>,<0>,<ADR>,<CHK>
					uint16_t high = payload.ReadByte();					

					/**
					DATA return <E7>, is SLOT#, DATA that ADR was found in
						; IF ADR not found, MASTER puts ADR in FREE slot
						; andsends DATA / STATUS return <E7>......
						; IF no FREE slot, Fail LACK, 0 is returned[<B4>, <3F>, <0>, <CHK>]
					*/

					uint16_t low = payload.ReadByte();					

					uint16_t address = (high << 7) + low;

					auto slot = m_clSlotManager.AcquireLocomotive(DccAddress{ address }, ticks);
					if (!slot)
					{
						Log::Error("[LoconetServiceImpl::ParseMessage] OPC_LOCO_ADR: No free slot for address {}", address);

						this->DispatchLnLongAckMessage(OPC_ERROR_LOCO_ADR, 0);
					}
					else
					{
						auto msg = m_clSlotManager.MakeMessage_SlotReadData(slot.value());

						this->DispatchLnMessage(msg);
						this->NotifySlotChanged(slot.value());
					}
				}
				break;

			case Opcodes::OPC_SL_RD_DATA:
				{
#if 0						
					uint8_t slot = *msg;

					if (slot >= MAX_SLOTS)
						//ignore for now
						return;

					//ignore
					return;

					Log::Trace("[RD_DATA] Slot: {}", slot);
					auto msg = this->MakeSlotReadDataMsg(slot);

					this->DispatchLnMessage(msg);
#endif
				}
				break;

			case Opcodes::OPC_LONG_ACK:
			case Opcodes::OPC_UNDOC_SETMS100:
			case Opcodes::OPC_PANEL_RESPONSE:
			case Opcodes::OPC_UNDOC_PANEL_QUERY:
				//ignore
				break;

			default:
				Log::Warn("[LoconetServiceImpl::Update] Unknow opcode: {:#x}", opcode);
				break;

		}
	}

	void LoconetServiceImpl::Update(const dcclite::Clock& clock)
	{	
		auto ticks = clock.Ticks();
		if (m_tNextPurgeTicks <= clock.Ticks())
		{
			Log::Trace("[LoconetServiceImpl::Update] Purging slots");
			
			m_clSlotManager.PurgeSlots(ticks, [this](uint8_t slot)
				{
					this->NotifySlotChanged(slot);
				}
			);

			m_tNextPurgeTicks = clock.Ticks() + PURGE_INTERVAL;						
		}

		//pump outgoing messages
		m_clMessageDispatcher.Update(m_clSerialPort);

		//Do we have any incoming message?
		if (!m_clInputPacket.IsDataReady())
			return;			

		auto numBytesRead = m_clInputPacket.GetDataSize();	
		if (numBytesRead)
		{					
			auto msg = m_clInputPacket.GetData();

			while (numBytesRead)
			{
				uint8_t opcode = *msg;
				uint8_t msgLen = 2;

				if ((opcode & 0x60) == 0x60)
				{
					msgLen = msg[1];				
				}
				else if (opcode & 0x20)
					msgLen = 4;
				else if (opcode & 0x40)
					msgLen = 6;		

				uint8_t checkSum = 0xFF;
				for (int i = 0; i < msgLen - 1; ++i)
				{
					checkSum ^= msg[i];
				}

				if (checkSum != msg[msgLen - 1])
				{
					Log::Warn("[LoconetServiceImpl::Update] Checksum mismatch, ignoring message");

					return;
				}

				auto nextMsg = msg + msgLen;

				//skip size byte
				if (msgLen > 6)
					++msg;

				++msg;		

				MiniPacket_t packet(msg, nextMsg - msg);

				this->ParseMessage(opcode, packet, ticks);
			
				msg = nextMsg;
				numBytesRead -= msgLen;
			}	
		}

		//grab more data
		m_clSerialPort.Read(m_clInputPacket);
	}

	void LoconetServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		LoconetService::Serialize(stream);

		m_clSlotManager.Serialize(stream);
	}

	void LoconetServiceImpl::NotifySlotChanged(uint8_t slotIndex) const
	{		
		this->NotifyItemChanged(*this,
			[this, slotIndex](JsonOutputStream_t &stream)
			{
				this->SerializeIdentification(stream);

				auto data = stream.AddObject("slot");
				data.AddIntValue("index", slotIndex);

				{
					auto slotData = data.AddObject("data");

					this->m_clSlotManager.SerializeSlot(slotIndex, slotData);
				}				
			}
		);
	}

	LoconetService::LoconetService(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project) :
		Service(name, broker, params, project)
	{
		//empty
	}

	std::unique_ptr<Service> LoconetService::Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project)
	{
		return std::make_unique<LoconetServiceImpl>(name, broker, params, project);
	}


}