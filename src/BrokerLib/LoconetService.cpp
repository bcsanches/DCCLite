#include "LoconetService.h"

#include <Log.h>

#include <Windows.h>

#include <optional>

#include "DccAddress.h"

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
	OPC_LOCO_SPD = 0xA0,
	OPC_LONG_ACK = 0xB4,
	OPC_MOVE_SLOTS = 0xBA,
	OPC_RQ_SL_DATA = 0xBB,
	OPC_LOCO_ADR = 0xBF,
	OPC_SL_RD_DATA = 0xE7,
	OPC_WR_SL_DATA = 0xEF
};

constexpr auto MAX_LN_MESSAGE_LEN = 20;
constexpr auto MAX_SLOTS = 120;

constexpr auto SLOT_STAT_SPEED_STEPS_BITS = (BIT_0 | BIT_1);	//011 = send 128 speed mode packets

enum class SlotStatUsage : uint8_t
{
	FREE =		0x00,
	COMMON =	BIT_0,
	IDLE =		BIT_1,
	IN_USE =	BIT_0 | BIT_1
};

//    D5 D4
// 00 1  1    0000
constexpr auto SLOT_STAT_USAGE_MASK = 0x30;

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
constexpr auto SLOT_SPEED_DIR_BIT = BIT_5;
constexpr auto SLOT_SPEED_F0 = BIT_4;
constexpr auto SLOT_SPEED_F4 = BIT_3;
constexpr auto SLOT_SPEED_F3 = BIT_2;
constexpr auto SLOT_SPEED_F2 = BIT_1;
constexpr auto SLOT_SPEED_F1 = BIT_0;

uint8_t DefaultMsgSizes(const Opcodes opcode)
{
	switch(opcode)
	{
		case OPC_SL_RD_DATA:
			return 0x0E;

		case OPC_WR_SL_DATA:
			return 14;

		default:
			throw std::exception(fmt::format("[LoconetService::DefaultMsgSizes] Unknown opcode: {}", opcode).c_str());
	}
}

class LoconetMessageWriter
{
	public:
		LoconetMessageWriter(Opcodes opcode)
		{
			m_uBuffer[0] = opcode;
			++m_uIndex;

			m_uMsgLen = DefaultMsgSizes(opcode);
			if (m_uMsgLen > 6)
			{
				m_uBuffer[1] = m_uMsgLen;
				++m_uIndex;
			}
		}

		void WriteByte(const uint8_t byte)
		{
			if (m_uIndex >= m_uMsgLen - 1)
				throw std::exception(fmt::format("[LoconetService::WriteByte] Buffer overflow").c_str());

			m_uBuffer[m_uIndex] = byte & 0x7F;
			++m_uIndex;
		}

		uint8_t *PackMsg()
		{
			uint8_t checksum = 0xFF;

			for (int i = 0; i < m_uMsgLen - 1; ++i)
			{
				checksum ^= m_uBuffer[i];
			}

			m_uBuffer[m_uMsgLen - 1] = checksum;

			return m_uBuffer;
		}

		uint8_t GetMsgLen() const
		{
			return m_uMsgLen;
		}

	private:
		uint8_t m_uBuffer[MAX_LN_MESSAGE_LEN];

		uint8_t m_uIndex = 0;
		uint8_t m_uMsgLen = 2;
};

struct Slot
{
	dcclite::broker::DccAddress m_LocomotiveAddress;

	uint8_t m_u8Stat = SLOT_STAT_SPEED_STEPS_BITS;

	//Set default as forward and directional lighting
	uint8_t m_u8Dirf = SLOT_SPEED_DIR_BIT | SLOT_SPEED_F0; 	

	bool IsFree() const
	{
		return this->GetSlotUsageBits() == SlotStatUsage::FREE;
	}	

	bool IsInUse() const
	{
		return this->GetSlotUsageBits() == SlotStatUsage::IN_USE;
	}

	void SetSlotUsage(SlotStatUsage usage)
	{
		if (usage == this->GetSlotUsageBits())
			return;

		uint8_t flags = static_cast<uint8_t>(usage) << 4;

		m_u8Stat = (m_u8Stat & (~SLOT_STAT_USAGE_MASK)) | flags;
	}

	private:
		inline SlotStatUsage GetSlotUsageBits() const
		{
			//BITS D4 D5 - 00110000
			return static_cast<SlotStatUsage>((m_u8Stat >> 4) & 0x03);
		}

		
};

class SlotManager
{
	public:
		SlotManager();

		std::optional<uint8_t> AcquireLocomotive(const dcclite::broker::DccAddress address);

		const Slot &GetSlot(uint8_t slot) const;	

		void SetSlotToInUse(uint8_t slot);

	private:
		std::array<Slot, MAX_SLOTS> m_arSlots;
};

SlotManager::SlotManager()
{	
	for (auto it : m_arSlots)
	{
		it.SetSlotUsage(SlotStatUsage::FREE);
	}

	//dispatch slot - never use
	m_arSlots[0].SetSlotUsage(SlotStatUsage::IN_USE);
}

std::optional<uint8_t> SlotManager::AcquireLocomotive(const dcclite::broker::DccAddress address)
{
	auto it = std::find_if(m_arSlots.begin(), m_arSlots.end(), [address](Slot &slot)
		{			
			return !slot.IsFree() && !slot.IsInUse() && (slot.m_LocomotiveAddress == address);
		}
	);

	if (it == m_arSlots.end())
	{
		it = std::find_if(m_arSlots.begin(), m_arSlots.end(), [](Slot &slot) {return slot.IsFree(); });
		if (it == m_arSlots.end())
			return false;

		it->SetSlotUsage(SlotStatUsage::COMMON);
		it->m_LocomotiveAddress = address;
	}

	return it - m_arSlots.begin();
}

const Slot &SlotManager::GetSlot(uint8_t slot) const
{	
	return m_arSlots.at(slot);
}

void SlotManager::SetSlotToInUse(uint8_t slot)
{
	m_arSlots.at(slot).SetSlotUsage(SlotStatUsage::IN_USE);
}

namespace dcclite::broker
{
	class LoconetServiceImpl : public LoconetService
	{
		public:
			LoconetServiceImpl(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);
			~LoconetServiceImpl() override;

			void Update(const dcclite::Clock &clock) override;

			void Initialize() override;

			static std::unique_ptr<Service> Create(const std::string &name, Broker &broker, const rapidjson::Value &params, const Project &project);

		private:
			LoconetMessageWriter MakeSlotReadDataMsg(uint8_t slot);

			void DispatchLnMessage(LoconetMessageWriter &msg);

		private:
			SlotManager m_clSlotManager;

			std::string m_strComPortName;

			HANDLE m_hComPort;
	};


	LoconetServiceImpl::LoconetServiceImpl(const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
		LoconetService(name, broker, params, project),
		m_strComPortName(params["port"].GetString())
	{		
		m_hComPort =  ::CreateFile(
			m_strComPortName.c_str(),
			GENERIC_READ | GENERIC_WRITE,
			0,
			0,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			0
		);

		if (m_hComPort == INVALID_HANDLE_VALUE)
			throw std::exception(fmt::format("[LoconetService] Error opening port {}", m_strComPortName).c_str());

		DCB dcb;

		if (!GetCommState(m_hComPort, &dcb))
		{
			CloseHandle(m_hComPort);

			throw std::exception(fmt::format("[LoconetService] Cannot read DCB data from {}", m_strComPortName).c_str());
		}

		dcb.fDtrControl = DTR_CONTROL_ENABLE;
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		dcb.fOutxDsrFlow = FALSE;
		dcb.fOutxCtsFlow = FALSE;
		dcb.StopBits = ONESTOPBIT;
		dcb.Parity = NOPARITY;
		dcb.BaudRate = CBR_57600;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;

		if (!SetCommState(m_hComPort, &dcb))
		{
			CloseHandle(m_hComPort);

			throw std::exception(fmt::format("[LoconetService] Cannot update CommState for {}", m_strComPortName).c_str());
		}

		dcclite::Log::Info("[LoconetService] Started, listening on port {}", m_strComPortName);
	}
	

	LoconetServiceImpl::~LoconetServiceImpl()
	{
		CloseHandle(m_hComPort);
	}

	void LoconetServiceImpl::Initialize()
	{
		//empty
	}

	LoconetMessageWriter LoconetServiceImpl::MakeSlotReadDataMsg(uint8_t slotIndex)
	{
		auto &slot = m_clSlotManager.GetSlot(slotIndex);

		//<0xE7>,<0E>,<SLOT#>,<STAT>,<ADR>,<SPD>,<DIRF>,<TRK> <SS2>, <ADR2>, <SND>, <ID1>, <ID2>, <CHK>
		LoconetMessageWriter msg(OPC_SL_RD_DATA);

		const auto rawLocoAddress = slot.m_LocomotiveAddress.GetAddress();

		msg.WriteByte(slotIndex);
		msg.WriteByte(slot.m_u8Stat);					//STAT
		msg.WriteByte(rawLocoAddress & 0x7F);			//ADDR
		msg.WriteByte(0);								//SPD
		msg.WriteByte(slot.m_u8Dirf);					//DIRF
		msg.WriteByte(0x01);							//TRK
		msg.WriteByte(0);								//SS2
		msg.WriteByte((rawLocoAddress >> 7) & 0x7F);	//ADDR2
		msg.WriteByte(0);								//SND
		msg.WriteByte(0);								//Id1
		msg.WriteByte(0);								//Id2

		return msg;
	}

	void LoconetServiceImpl::DispatchLnMessage(LoconetMessageWriter &msg)
	{
		auto *data = msg.PackMsg();

		auto len = msg.GetMsgLen();

		DWORD bytesWritten = 0;		
		if (!WriteFile(m_hComPort, msg.PackMsg(), len, &bytesWritten, nullptr))
		{
			Log::Error("[LoconetServiceImpl::DispatchLnMessage] Error writing: {}", GetLastError());
		}

		{
			auto msgData = msg.PackMsg();

			uint8_t opcode = *msgData;
			uint8_t msgLen = 2;

			if ((opcode & 0x60) == 0x60)
			{
				msgLen = msgData[1];
			}
			else if (opcode & 0x20)
				msgLen = 4;
			else if (opcode & 0x40)
				msgLen = 6;

			uint8_t checkSum = 0xFF;
			for (int i = 0; i < msgLen - 1; ++i)
			{
				checkSum ^= msgData[i];
			}

			if (checkSum != msgData[msgLen - 1])
			{
				Log::Warn("[LoconetServiceImpl::Update] Checksum mismatch, ignoring message");

				return;
			}
		}
	}

	void LoconetServiceImpl::Update(const dcclite::Clock& clock)
	{
		BYTE buffer[1024];

		DWORD dwBytesRead;

		if (!ReadFile(m_hComPort, buffer, sizeof(buffer), &dwBytesRead, nullptr)) 
		{
			Log::Error("Error: {}", GetLastError());
			return;
		}

		if (!dwBytesRead)
			return;
		
		uint8_t *msg = buffer;		

		while (dwBytesRead)
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

			switch (opcode)
			{
				case Opcodes::OPC_LOCO_SPD:
					{
						uint8_t slot = *msg;
						++msg;

						uint8_t speed = *msg;
						++msg;

						Log::Trace("[Speed] {}", speed);
					}
					break;

				case Opcodes::OPC_MOVE_SLOTS:
					{
						uint8_t src = *msg;
						++msg;

						uint8_t dest = *msg;
						++msg;

						//is a null move)
						if (src == dest)
						{
							m_clSlotManager.SetSlotToInUse(src);

							auto msg = this->MakeSlotReadDataMsg(src);

							this->DispatchLnMessage(msg);
						}
						else
						{
							Log::Trace("Ops");
						}
					}
					break;

				case Opcodes::OPC_RQ_SL_DATA:
					{
						uint8_t slot = *msg;

						auto msg = this->MakeSlotReadDataMsg(slot);

						this->DispatchLnMessage(msg);																
					}
					break;

				case Opcodes::OPC_LOCO_ADR:
					{
						//<0xBF>,<0>,<ADR>,<CHK>
						uint16_t high = *msg;
						++msg; 

						/**
						DATA return <E7>, is SLOT#, DATA that ADR was found in
							; IF ADR not found, MASTER puts ADR in FREE slot
							; andsends DATA / STATUS return <E7>......
							; IF no FREE slot, Fail LACK, 0 is returned[<B4>, <3F>, <0>, <CHK>]
						*/

						uint16_t low = *msg;
						++msg;

						uint16_t address = (high << 7) + low;

						auto slot = m_clSlotManager.AcquireLocomotive(DccAddress{ address });
						if (!slot)
						{
							LoconetMessageWriter msg{ OPC_LONG_ACK };
							msg.WriteByte(0x3F);
							msg.WriteByte(0x00);

							this->DispatchLnMessage(msg);
						}
						else
						{
							auto msg = this->MakeSlotReadDataMsg(slot.value());

							this->DispatchLnMessage(msg);
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

				default:
					Log::Trace("Ops");
					break;

			}
			
			msg = nextMsg;
			dwBytesRead -= msgLen;			
		}


		//do something
		Log::Trace("Got {} bytes", dwBytesRead);
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