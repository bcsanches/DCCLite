// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DccTerminalCmds.h"

#include <fstream>
#include <future>

#include <fmt/format.h>

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>
#include <dcclite/RName.h>

#include <exec/dcc/DccLiteService.h>
#include <exec/dcc/IResettableObject.h>
#include <exec/dcc/NetworkDevice.h>
#include <exec/dcc/OutputDecoder.h>
#include <exec/dcc/SignalDecoder.h>

#include <sys/Project.h>
#include <sys/ServiceFactory.h>

#include "CmdHostService.h"
#include "DeviceClearEEPromCmd.h"
#include "DeviceNetworkTestCmds.h"
#include "DeviceRenameCmd.h"
#include "ServoProgrammerCmds.h"
#include "TerminalCmd.h"
#include "TerminalContext.h"
#include "TerminalUtils.h"

using namespace dcclite;
using namespace dcclite::broker;
using namespace dcclite::broker::shell::terminal;

using dcclite::broker::shell::terminal::detail::GetCurrentFolder;
using dcclite::broker::shell::terminal::detail::GetNetworkDevice;
using dcclite::broker::shell::terminal::detail::MakeRpcErrorResponse;
using dcclite::broker::shell::terminal::detail::MakeRpcResultMessage;


/////////////////////////////////////////////////////////////////////////////
//
// ResetItemCmd
//
/////////////////////////////////////////////////////////////////////////////
class ResetItemCmd : public TerminalCmd
{
	public:
		explicit ResetItemCmd(RName name = RName{ "Reset-Item" }) :
			TerminalCmd(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto paramsIt = request.FindMember("params");
			if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
			{
				throw TerminalCmdException(fmt::format("Usage: {} <item_path>", this->GetName()), id);
			}

			auto itemPath = paramsIt->value[0].GetString();

			auto obj = dynamic_cast<IFolderObject *>(context.TryGetItem());
			if (!obj)
			{
				throw TerminalCmdException("Terminal is not on a valid location", id);
			}

			auto ireset = dynamic_cast<IResettableObject *>(obj->TryNavigate(Path_t{ itemPath }));
			if (!ireset)
			{
				throw TerminalCmdException(fmt::format("Service at {} it not resetable or not found", itemPath), id);
			}

			dcclite::Log::Info("[ResetCmd] Resetting {}.", itemPath);
			ireset->Reset();

			return MakeRpcResultMessage(id, [](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", "OK");
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// RebootDeviceCmd
//
/////////////////////////////////////////////////////////////////////////////
class RebootDeviceCmd : public TerminalCmd
{
	public:
		explicit RebootDeviceCmd(RName name = RName{ "Reboot-Device" }) :
			TerminalCmd(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto paramsIt = request.FindMember("params");
			if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
			{
				throw TerminalCmdException(fmt::format("Usage: {} <NetworkDevicePath>", this->GetName()), id);
			}

			auto path = paramsIt->value[0].GetString();

			auto &device = GetNetworkDevice(dcclite::Path_t{ path }, context, id);

			device.ResetRemoteDevice();

			return MakeRpcResultMessage(id, [](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", "OK");
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// DisconnectDeviceCmd
//
/////////////////////////////////////////////////////////////////////////////
class DisconnectDeviceCmd : public TerminalCmd
{
	public:
		explicit DisconnectDeviceCmd(RName name = RName{ "Disconnect-Device" }) :
			TerminalCmd(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto paramsIt = request.FindMember("params");
			if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
			{
				throw TerminalCmdException(fmt::format("Usage: {} <NetworkDevicePath>", this->GetName()), id);
			}

			auto path = paramsIt->value[0].GetString();

			auto &device = GetNetworkDevice(dcclite::Path_t{ path }, context, id);

			device.DisconnectDevice();

			return MakeRpcResultMessage(id, [](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", "OK");
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// BlockDeviceCmd
//
/////////////////////////////////////////////////////////////////////////////
class BlockDeviceCmd : public TerminalCmd
{
	public:
		explicit BlockDeviceCmd(RName name = RName{ "Block-Device" }) :
			TerminalCmd(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto paramsIt = request.FindMember("params");
			if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
			{
				throw TerminalCmdException(fmt::format("Usage: {} <NetworkDevicePath>", this->GetName()), id);
			}

			auto path = paramsIt->value[0].GetString();

			auto &device = GetNetworkDevice(dcclite::Path_t{ path }, context, id);

			device.Block();

			return MakeRpcResultMessage(id, [](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", "OK");
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// ClearDccLiteBlockList
//
/////////////////////////////////////////////////////////////////////////////
class ClearDccLiteBlockListCmd : public TerminalCmd
{
	public:
		explicit ClearDccLiteBlockListCmd(RName name = RName{ "Clear-DccLiteBlockList" }) :
			TerminalCmd(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto paramsIt = request.FindMember("params");
			if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
			{
				throw TerminalCmdException(fmt::format("Usage: {} <DccLiteService>", this->GetName()), id);
			}

			auto path = paramsIt->value[0].GetString();

			auto &folder = GetCurrentFolder(context, id);

			auto obj = folder.TryNavigate(Path_t{ path });
			if (obj == nullptr)
			{
				throw TerminalCmdException(fmt::format("Object {} not found", path), id);
			}

			auto dccLiteService = dynamic_cast<exec::dcc::DccLiteService *>(obj);
			if (dccLiteService == nullptr)
			{
				throw TerminalCmdException(fmt::format("Object {} is not an DccLiteService", path), id);
			}

			dccLiteService->ClearBlockList();

			return MakeRpcResultMessage(id, [](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", "OK");
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// DecoderCmdBase
//
/////////////////////////////////////////////////////////////////////////////
class DecoderCmdBase : public TerminalCmd
{
	protected:
		explicit DecoderCmdBase(RName name) :
			TerminalCmd(name)
		{
			//empty
		}

		exec::dcc::Decoder &FindDecoder(const TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
		{
			auto paramsIt = request.FindMember("params");
			if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 1))
			{
				throw TerminalCmdException(fmt::format("Usage: {} <decoder_path>", this->GetName()), id);
			}

			auto folder = dynamic_cast<IFolderObject *>(context.TryGetItem());
			if (folder == nullptr)
			{
				throw TerminalCmdException(fmt::format("Currently location is invalid"), id);
			}

			auto path = Path_t{ paramsIt->value[0].GetString() };

			auto obj = folder->TryNavigate(path);
			if (obj == nullptr)
			{
				throw TerminalCmdException(fmt::format("Decoder not found: {}", path.string()), id);
			}

			auto decoder = dynamic_cast<exec::dcc::Decoder *>(obj);
			if (decoder == nullptr)
			{
				throw TerminalCmdException(fmt::format("Path does not lead to a decoder: {}", path.string()), id);
			}

			return *decoder;
		}

		exec::dcc::OutputDecoder *FindOutputDecoder(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request)
		{
			auto &decoder = this->FindDecoder(context, id, request);

			auto outputDecoder = dynamic_cast<exec::dcc::OutputDecoder *>(&decoder);
			if (outputDecoder == nullptr)
			{
				throw TerminalCmdException(fmt::format("Decoder {} is not an output type", decoder.GetName()), id);
			}

			return outputDecoder;
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// ActivateItemCmd
//
/////////////////////////////////////////////////////////////////////////////
class ActivateItemCmd : public DecoderCmdBase
{
	public:
		explicit ActivateItemCmd(RName name = RName{ "Activate-Item" }) :
			DecoderCmdBase(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto outputDecoder = this->FindOutputDecoder(context, id, request);

			outputDecoder->Activate("ActivateItemCmd");

			return MakeRpcResultMessage(id, [](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", "OK");
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// DeactivateItemCmd
//
/////////////////////////////////////////////////////////////////////////////
class DeactivateItemCmd : public DecoderCmdBase
{
	public:
		explicit DeactivateItemCmd(RName name = RName{ "Deactivate-Item" }) :
			DecoderCmdBase(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto outputDecoder = this->FindOutputDecoder(context, id, request);

			outputDecoder->Deactivate("DeactivateItemCmd");

			return MakeRpcResultMessage(id, [](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", "OK");
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// FlipItemCmd
//
/////////////////////////////////////////////////////////////////////////////
class FlipItemCmd : public DecoderCmdBase
{
	public:
		explicit FlipItemCmd(RName name = RName{ "Flip-Item" }) :
			DecoderCmdBase(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto outputDecoder = this->FindOutputDecoder(context, id, request);

			outputDecoder->ToggleState("FlipItemCmd");

			return MakeRpcResultMessage(id, [outputDecoder](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", fmt::format("OK: {}", dcclite::DecoderStateName(outputDecoder->GetRequestedState())));
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// SetAspectCmd
//
/////////////////////////////////////////////////////////////////////////////
class SetAspectCmd : public DecoderCmdBase
{
	public:
		explicit SetAspectCmd(RName name = RName{ "Set-Aspect" }) :
			DecoderCmdBase(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto &decoder = this->FindDecoder(context, id, request);

			auto signalDecoder = dynamic_cast<exec::dcc::SignalDecoder *>(&decoder);
			if (signalDecoder == nullptr)
			{
				throw TerminalCmdException(fmt::format("Decoder {} on DCC System is not an Signal type", decoder.GetName()), id);
			}

			auto paramsIt = request.FindMember("params");
			if (paramsIt->value.Size() < 2)
			{
				throw TerminalCmdException(fmt::format("Usage: {} <decoder_path> <aspect>", this->GetName()), id);
			}

			auto aspectName = paramsIt->value[1].GetString();
			auto aspect = dcclite::TryConvertNameToAspect(aspectName);
			if (!aspect.has_value())
			{
				throw TerminalCmdException(fmt::format("Invalid aspect name {}", aspectName), id);
			}

			signalDecoder->SetAspect(aspect.value(), this->GetName().GetData().data(), "Json proc");

			return MakeRpcResultMessage(id, [aspectName](Result_t &results)
				{
					results.AddStringValue("classname", "string");
					results.AddStringValue("msg", fmt::format("OK: {}", aspectName));
				}
			);
		}
};

/////////////////////////////////////////////////////////////////////////////
//
// ReadEEPromCmd
//
/////////////////////////////////////////////////////////////////////////////
class ReadEEPromFiber : public TerminalCmdFiber, private exec::dcc::NetworkTask::IObserver, public sys::EventHub::IEventTarget
{
	public:
		ReadEEPromFiber(const CmdId_t id, TerminalContext &context, exec::dcc::NetworkDevice &device) :
			TerminalCmdFiber(id, context),
			m_spTask{ device.StartDownloadEEPromTask(this, m_vecEEPromData) }
		{
			if (!m_spTask)
				throw TerminalCmdException("No task provided for ReadEEPromFiber", id);

			std::string fileName{ device.GetName().GetData() };
			fileName.append(".rom");

			m_pathRomFileName = sys::Project::GetAppFilePath(fileName);
		}

		~ReadEEPromFiber()
		{
			//
			//Must wait, so we make sure the thread will not fire a event after it is destroyed
			if (m_Future.valid())
				m_Future.wait();

			sys::EventHub::CancelEvents(*this);
		}

	private:
		void OnNetworkTaskStateChanged(exec::dcc::NetworkTask &task) override
		{
			assert(&task == m_spTask.get());

			m_spTask->SetObserver(nullptr);

			if (m_spTask->HasFailed())
			{
				m_rclContext.SendClientNotification(MakeRpcErrorResponse(m_tCmdId, fmt::format("Download task failed: {}", m_spTask->GetMessage())));

				//suicide, we are useless now
				m_rclContext.DestroyFiber(*this);

				return;
			}

			if (m_spTask->HasFinished())
			{
				//we do not need the task anymore
				m_spTask.reset();

				//save the data to disk...
				m_Future = std::async(SaveEEprom, std::ref(*this), std::ref(m_vecEEPromData), std::ref(m_pathRomFileName));
			}
		}

		class DiskWriteFinishedEvent : public sys::EventHub::IEvent
		{
			public:
				explicit DiskWriteFinishedEvent(ReadEEPromFiber &target) :
					IEvent{ target }
				{
					//empty
				}

				void Fire() override
				{
					static_cast<ReadEEPromFiber &>(this->GetTarget()).OnSaveEEPromFinished();
				}
		};

		void OnSaveEEPromFinished()
		{
			auto msg = MakeRpcResultMessage(m_tCmdId, [this](Result_t &results)
				{
					results.AddStringValue("classname", "ReadEEPromResult");
					results.AddStringValue("filepath", m_pathRomFileName.string());
				}
			);

			m_rclContext.SendClientNotification(msg);

			//finally finished, so go away like MR. MEESEEKS
			m_rclContext.DestroyFiber(*this);
		}

		static void SaveEEprom(ReadEEPromFiber &fiber, const exec::dcc::DownloadEEPromTaskResult_t &data, const dcclite::fs::path &fileName)
		{
			dcclite::Log::Info("[ReadEEPromFiber::SaveEEprom] Saving EEPROM at {}", fileName.string());

			std::ofstream epromFile(fileName, std::ios::binary);
			if (!epromFile)
			{
				dcclite::Log::Error("[ReadEEPromFiber::SaveEEprom] cannot create {}", fileName.string());

				return;
			}

			epromFile.write(reinterpret_cast<const char *>(&data.front()), data.size());
			dcclite::Log::Info("[ReadEEPromFiber::SaveEEprom] Finished saving EEPROM at {}", fileName.string());

			//
			//Send results.. but we need to do this on main thread
			sys::EventHub::PostEvent< DiskWriteFinishedEvent>(std::ref(fiber));
		}

	private:
		exec::dcc::DownloadEEPromTaskResult_t m_vecEEPromData;

		std::shared_ptr<exec::dcc::NetworkTask> m_spTask;

		dcclite::fs::path m_pathRomFileName;

		std::future<void> m_Future;
};

class ReadEEPromCmd : public TerminalCmd
{
	public:
		explicit ReadEEPromCmd(RName name = RName{ "Read-EEProm" }) :
			TerminalCmd(name)
		{
			//empty
		}

		CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
		{
			auto paramsIt = request.FindMember("params");
			if (paramsIt->value.Size() < 1)
			{
				throw TerminalCmdException(fmt::format("Usage: {} <itemPath>", this->GetName()), id);
			}

			auto locationParam = paramsIt->value[0].GetString();

			auto &networkDevice = GetNetworkDevice(dcclite::Path_t(locationParam), context, id);

			return std::make_unique<ReadEEPromFiber>(id, context, networkDevice);
		}
};

namespace dcclite::broker::shell::terminal
{
	const char *DccTerminalCmdsInitService::TYPE_NAME = "DccTerminalCmdsInitService";

	DccTerminalCmdsInitService::DccTerminalCmdsInitService(RName name, sys::Broker &broker, const rapidjson::Value &params, CmdHostService &cmdHost):
		InitService{ name, broker, params }
	{
		{
			cmdHost.AddCmd(std::make_unique<ActivateItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<DeactivateItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<FlipItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<StartServoProgrammerCmd>());
			cmdHost.AddCmd(std::make_unique<StopServoProgrammerCmd>());
			cmdHost.AddCmd(std::make_unique<EditServoProgrammerCmd>());
			cmdHost.AddCmd(std::make_unique<DeployServoProgrammerCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<SetAspectCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<ReadEEPromCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<ResetItemCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<BlockDeviceCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<ClearDccLiteBlockListCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<DisconnectDeviceCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<RebootDeviceCmd>());
		}

		{
			cmdHost.AddCmd(std::make_unique<ClearEEPromCmd>());
		}

		{
			auto renameItemCmd = cmdHost.AddCmd(std::make_unique<RenameItemCmd>());

			cmdHost.AddAlias(RName{ "ren" }, *renameItemCmd);
		}

		{
			cmdHost.AddCmd(std::make_unique<StartNetworkTestCmd>());
			cmdHost.AddCmd(std::make_unique<StopNetworkTestCmd>());
			cmdHost.AddCmd(std::make_unique<ReceiveNetworkTestDataCmd>());
		}
	}

	void DccTerminalCmdsInitService::RegisterFactory()
	{
		static sys::GenericServiceWithDependenciesFactory<DccTerminalCmdsInitService> g_clDccTerminalCmdsInitService;
	}
}
