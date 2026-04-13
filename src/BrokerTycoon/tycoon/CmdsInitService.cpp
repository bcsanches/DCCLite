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

	/////////////////////////////////////////////////////////////////////////////
	//
	// ResetItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class SetIndustrySpotReserved : public terminal::TerminalCmd
	{
		public:
			explicit SetIndustrySpotReserved(RName name = RName{ "Set-IndustrySpotReserved" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(terminal::TerminalContext &context, const terminal::CmdId_t id, const rapidjson::Document &request) override
			{				
				auto paramsIt = request.FindMember("params");
				if ((paramsIt == request.MemberEnd()) || (!paramsIt->value.IsArray()) || (paramsIt->value.Size() < 2))
				{
					throw terminal::TerminalCmdException(fmt::format("Usage: {} <industry_path> <spot> [information]", this->GetName()), id);
				}

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

				const char *info = nullptr;
				if (paramsIt->value.Size() == 3)
				{
					info = paramsIt->value[2].GetString();
					dcclite::Log::Info("[SetIndustrySpotReserved] Reserving {} for {}.", itemPath, info);
				}
				else
				{
					dcclite::Log::Info("[SetIndustrySpotReserved] Reserving {}.", itemPath);
				}

				industry->ReserveSpot(paramsIt->value[1].GetString(), info);

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
	// TerminalCmdsInitService
	//
	/////////////////////////////////////////////////////////////////////////////

	const char *TerminalCmdsInitService::TYPE_NAME = "TycoonTerminalCmdsInitService";

	TerminalCmdsInitService::TerminalCmdsInitService(RName name, sys::Broker &broker, const rapidjson::Value &params, shell::terminal::CmdHostService &cmdHost) :
		InitService{ name, broker, params }
	{
		{
			cmdHost.AddCmd(std::make_unique<SetIndustrySpotReserved>());
		}
	}

	void TerminalCmdsInitService::RegisterFactory()
	{
		static sys::GenericServiceWithDependenciesFactory<TerminalCmdsInitService> g_clCmdsInitService;
	}
}
