// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "CmdsInitService.h"

#include <fmt/format.h>

#include <dcclite/FmtUtils.h>

#include <sys/ServiceFactory.h>

#include <shell/terminal/CmdHostService.h>
#include <shell/terminal/TerminalContext.h>
#include <shell/terminal/TerminalCmd.h>
#include <shell/terminal/TerminalUtils.h>

#include "Industry.h"

namespace dcclite::broker::tycoon
{
	namespace terminal = shell::terminal;

	class IndustrySpotCmd : public terminal::TerminalCmd
	{
		protected:
			explicit IndustrySpotCmd(RName name) :
				TerminalCmd(name)
			{
				//empty
			}

			virtual size_t GetNumParams() const noexcept
			{
				return 2;
			}

			virtual std::string_view GetParamsMsg() const noexcept
			{				
				return "<industry_path> <spot>";
			}

			virtual void RunInternal(Industry &target, RName spotName, const rapidjson::Value &array, const terminal::CmdId_t id) = 0;

		public:
			CmdResult_t Run(terminal::TerminalContext &context, const terminal::CmdId_t id, const rapidjson::Document &request) override
			{
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < this->GetNumParams()))
				{
					throw terminal::TerminalCmdException(fmt::format("Usage: {} {}", this->GetName(), this->GetParamsMsg()), id);
				}
				paramsIt->value.GetArray();

				auto itemPath = paramsIt->value[0].GetString();

				auto obj = dynamic_cast<IFolderObject *>(context.TryGetItem());
				if (!obj)
				{
					throw terminal::TerminalCmdException("Terminal is not on a valid location", id);
				}

				auto industry = dynamic_cast<Industry *>(obj->TryNavigate(Path_t{ itemPath }));
				if (!industry)
				{
					throw terminal::TerminalCmdException(fmt::format("Industry at {} not found", itemPath), id);
				}

				auto spotNameStr = std::string_view(paramsIt->value[1].GetString(), paramsIt->value[1].GetStringLength());
				RName spotName = RName::TryGetName(spotNameStr);
				if(!spotName)
				{
					throw terminal::TerminalCmdException(fmt::format("Invalid spot name: {}", spotNameStr), id);
				}

				try
				{
					this->RunInternal(*industry, spotName, paramsIt->value, id);
				}
				catch(terminal::TerminalCmdException &)
				{
					//just to avoid next catch catching it..
					throw;
				}
				catch (std::exception &ex)
				{
					throw terminal::TerminalCmdException(ex.what(), id);
				}

				return terminal::MsgUtils::MakeRpcResultMessage(id, [](Result_t &results)
					{
						results.AddStringValue("classname", "string");
						results.AddStringValue("msg", "OK");
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// SetIndustrySpotReserved
	//
	/////////////////////////////////////////////////////////////////////////////
	class SetIndustrySpotReservedCmd : public IndustrySpotCmd
	{
		public:
			explicit SetIndustrySpotReservedCmd(RName name = RName{ "Set-IndustrySpotReserved" }) :
				IndustrySpotCmd(name)
			{
				//empty
			}

		protected:
			std::string_view GetParamsMsg() const noexcept override
			{
				return "<industry_path> <spot> [information]";
			}

			void RunInternal(Industry &target, RName spotName, const rapidjson::Value &array, const terminal::CmdId_t id) override
			{				
				const char *info = nullptr;
				if (array.Size() == 3)
				{
					info = array[2].GetString();
					dcclite::Log::Info("[SetIndustrySpotReserved] Reserving {} for {}.", target.GetPath().string(), info);
				}
				else
				{
					dcclite::Log::Info("[SetIndustrySpotReserved] Reserving {}.", target.GetPath().string());
				}

				target.ReserveSpot(spotName, info);						
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// ClearIndustrySpotReservationCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class ClearIndustrySpotReservationCmd : public IndustrySpotCmd
	{
		public:
			explicit ClearIndustrySpotReservationCmd(RName name = RName{ "Clear-IndustrySpotReservation" }) :
				IndustrySpotCmd(name)
			{
				//empty
			}

		protected:			
			void RunInternal(Industry &target, RName spotName, const rapidjson::Value &array, const terminal::CmdId_t id) override
			{				
				target.CancelSpotReservation(spotName);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// StartIndustrySpotLoadCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class StartIndustrySpotLoadCmd : public IndustrySpotCmd
	{
		public:
			explicit StartIndustrySpotLoadCmd(RName name = RName{ "Start-IndustrySpotLoad" }) :
				IndustrySpotCmd(name)
			{
				//empty
			}

		protected:
			std::string_view GetParamsMsg() const noexcept override
			{
				return "<industry_path> <spot> <cargo>";
			}

			size_t GetNumParams() const noexcept override
			{
				return 3;
			}

			void RunInternal(Industry &target, RName spotName, const rapidjson::Value &array, const terminal::CmdId_t id) override
			{
				auto cargoNameStr = json::MakeStringView(array[2]);
				RName cargoName = RName::TryGetName(cargoNameStr);

				if (!cargoName)
				{
					throw terminal::TerminalCmdException(fmt::format("Invalid cargo name: {}", cargoNameStr), id);
				}

				target.StartSpotLoad(spotName, cargoName);
			}			
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// RemoveCarFromIndustrySpotCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class RemoveCarFromIndustrySpotCmd : public IndustrySpotCmd
	{
		public:
			explicit RemoveCarFromIndustrySpotCmd(RName name = RName{ "Remove-CarFromSpot" }) :
				IndustrySpotCmd(name)
			{
				//empty
			}

		protected:
			void RunInternal(Industry &target, RName spotName, const rapidjson::Value &array, const terminal::CmdId_t id) override
			{
				target.RemoveCarFromSpot(spotName);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// TerminalCmdsInitService
	//
	/////////////////////////////////////////////////////////////////////////////

	const char *TerminalCmdsInitService::TYPE_NAME = "TycoonTerminalCmdsInitService";

	TerminalCmdsInitService::TerminalCmdsInitService(RName name, sys::Broker &broker, const rapidjson::Value &params, shell::terminal::CmdHostService &cmdHost) :
		InitService{ name, broker, params }
	{		
		cmdHost.AddCmd(std::make_unique<SetIndustrySpotReservedCmd>());
		cmdHost.AddCmd(std::make_unique<ClearIndustrySpotReservationCmd>());
		cmdHost.AddCmd(std::make_unique<StartIndustrySpotLoadCmd>());
		cmdHost.AddCmd(std::make_unique<RemoveCarFromIndustrySpotCmd>());		
	}

	void TerminalCmdsInitService::RegisterFactory()
	{
		static sys::GenericServiceWithDependenciesFactory<TerminalCmdsInitService> g_clCmdsInitService;
	}
}
