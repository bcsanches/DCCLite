// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.
//

#include "TerminalServiceCmds.h"

#include <dcclite/FmtUtils.h>

#include "CmdHostService.h"
#include "TerminalContext.h"
#include "TerminalUtils.h"

#include <fmt/format.h>

namespace dcclite::broker::shell::terminal
{
	/////////////////////////////////////////////////////////////////////////////
	//
	// GetChildItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetChildItemCmd : public TerminalCmd
	{
		public:
			explicit GetChildItemCmd(RName name = RName{ "Get-ChildItem" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto folder = &detail::GetCurrentFolder(context, id);

				auto paramsIt = request.FindMember("params");
				if (paramsIt != request.MemberEnd())
				{
					auto locationParam = paramsIt->value[0].GetString();
					auto item = folder->TryNavigate(dcclite::Path_t(locationParam));
					if (!item)
					{
						throw TerminalCmdException(fmt::format("Invalid location {}", locationParam), id);
					}

					if (!item->IsFolder())
					{
						throw TerminalCmdException(fmt::format("Location is not a folder {}", locationParam), id);
					}

					folder = static_cast<IFolderObject *>(item);
				}

				return detail::MakeRpcResultMessage(id, [folder](Result_t &results)
					{
						results.AddStringValue("classname", "ChildItem");
						results.AddStringValue("location", folder->GetPath().string());

						auto dataArray = results.AddArray("children");

						folder->ConstVisitChildren([&dataArray](const IObject &item)
							{
								auto itemObject = dataArray.AddObject();
								item.Serialize(itemObject);

								return true;
							}
						);
					});
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// GetItemCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetItemCmd : public TerminalCmd
	{
		public:
			explicit GetItemCmd(RName name = RName{ "Get-Item" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto &folder = detail::GetCurrentFolder(context, id);

				auto paramsIt = request.FindMember("params");
				if (paramsIt == request.MemberEnd())
				{
					throw TerminalCmdException(fmt::format("Usage: {} <path>", this->GetName()), id);
				}

				auto locationParam = paramsIt->value[0].GetString();
				auto item = folder.TryNavigate(dcclite::Path_t(locationParam));
				if (!item)
				{
					throw TerminalCmdException(fmt::format("Invalid location {}", locationParam), id);
				}

				return detail::MakeRpcResultMessage(id, [item](Result_t &results)
					{
						results.AddStringValue("classname", "Item");
						results.AddStringValue("location", item->GetPath().string());

						auto dataObj = results.AddObject("item");
						item->Serialize(dataObj);
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// SetLocationCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class SetLocationCmd : public TerminalCmd
	{
		public:
			explicit SetLocationCmd(RName name = RName{ "Set-Location" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto &folder = detail::GetCurrentFolder(context, id);

				dcclite::Path_t path;

				auto paramsIt = request.FindMember("params");
				if (paramsIt != request.MemberEnd())
				{
					if (!paramsIt->value.IsArray())
					{
						throw TerminalCmdException("Expected positional parameters", id);
					}

					auto destinationPath = paramsIt->value[0].GetString();

					auto destinationObj = dynamic_cast<IFolderObject *>(folder.TryNavigate(Path_t(destinationPath)));
					if (!destinationObj)
					{
						throw TerminalCmdException(fmt::format("Invalid path {}", destinationPath), id);
					}

					context.SetLocation(*destinationObj);
				}
				else
				{
					path = folder.GetPath();
				}

				return detail::MakeRpcResultMessage(id, [&path](Result_t &results)
					{
						results.AddStringValue("classname", "Location");
						results.AddStringValue("location", path.string());
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// GetCommandCmd
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetCommandCmd : public TerminalCmd
	{
		public:
			explicit GetCommandCmd(RName name = RName{ "Get-Command" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto item = this->GetParent();

				assert(item->IsFolder());

				auto folder = static_cast<FolderObject *>(item);

				return detail::MakeRpcResultMessage(id, [folder](Result_t &results)
					{
						results.AddStringValue("classname", "CmdList");

						auto dataArray = results.AddArray("cmds");

						folder->ConstVisitChildren([&dataArray](auto &cmd)
							{
								auto itemObject = dataArray.AddObject();
								cmd.Serialize(itemObject);

								return true;
							}
						);
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// GetRNames
	//
	/////////////////////////////////////////////////////////////////////////////
	class GetRNames : public TerminalCmd
	{
		public:
			explicit GetRNames(RName name = RName{ "Get-RNames" }) :
				TerminalCmd(name)
			{
				//empty
			}

			CmdResult_t Run(TerminalContext &context, const CmdId_t id, const rapidjson::Document &request) override
			{
				auto names = dcclite::detail::RName_GetAll();

				return detail::MakeRpcResultMessage(id, [&names](Result_t &results)
					{
						results.AddStringValue("classname", "RNames");
						auto dataArray = results.AddArray("rnames");
						for (const auto &it : names)
						{
							auto obj = dataArray.AddObject();

							obj.AddStringValue("name", it.GetData());
							obj.AddIntValue("index", it.GetIndex());

							auto clusterInfo = it.FindClusterInfo();
							obj.AddIntValue("cluster", clusterInfo.first);
							obj.AddIntValue("position", clusterInfo.second);
						}
					}
				);
			}
	};

	/////////////////////////////////////////////////////////////////////////////
	//
	// RegisterBaseTerminalCmds
	//
	/////////////////////////////////////////////////////////////////////////////
	void RegisterBaseTerminalCmds(CmdHostService &cmdHost)
	{
		{
			auto getChildItemCmd = cmdHost.AddCmd(std::make_unique<GetChildItemCmd>());
			cmdHost.AddAlias(RName{ "dir" }, *getChildItemCmd);
			cmdHost.AddAlias(RName{ "ls" }, *getChildItemCmd);
		}

		{
			cmdHost.AddCmd(std::make_unique<GetItemCmd>());
		}

		{
			auto setLocationCmd = cmdHost.AddCmd(std::make_unique<SetLocationCmd>());
			cmdHost.AddAlias(RName{ "cd" }, *setLocationCmd);
		}

		{
			auto getCommandCmd = cmdHost.AddCmd(std::make_unique<GetCommandCmd>());
			cmdHost.AddAlias(RName{ "gcm" }, *getCommandCmd);
		}

		{
			cmdHost.AddCmd(std::make_unique<GetRNames>());
		}
	}
}


