// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "TerminalUtils.h"

#include <fmt/format.h>

#include "../dcc/NetworkDevice.h"

namespace dcclite::broker::detail
{			
	using namespace dcclite;

	std::string MakeRpcMessage(CmdId_t id, std::string_view *methodName, std::string_view nestedObjName, std::function<void(JsonOutputStream_t &object)> filler)
	{
		JsonCreator::StringWriter messageWriter;

		{
			auto messageObj = JsonCreator::MakeObject(messageWriter);

			messageObj.AddStringValue(JSONRPC_KEY, JSONRPC_VERSION);

			if (id >= 0)
			{
				messageObj.AddIntValue("id", id);
			}

			if (methodName)
				messageObj.AddStringValue("method", *methodName);

			if (filler)
			{
				auto params = messageObj.AddObject(nestedObjName);

				filler(params);
			}
		}

		return messageWriter.GetString();
	}

	IFolderObject &GetCurrentFolder(const TerminalContext &context, const CmdId_t id)
	{
		auto item = context.TryGetItem();
		if (!item || !item->IsFolder())
		{
			throw TerminalCmdException(fmt::format("Current location {} is invalid", context.GetLocation().string()), id);
		}

		return static_cast<IFolderObject &>(*item);
	}

	NetworkDevice &GetNetworkDevice(const dcclite::Path_t &path, const TerminalContext &context, const CmdId_t id)
	{
		auto &folder = GetCurrentFolder(context, id);

		auto item = folder.TryNavigate(path);
		if (item == nullptr)
		{
			throw TerminalCmdException(fmt::format("Invalid path {}", path.string()), id);
		}

		auto dev = dynamic_cast<NetworkDevice *>(item);
		if (dev == nullptr)
		{
			throw TerminalCmdException(fmt::format("Path does lead to a NetworkDevice: {}", path.string()), id);
		}

		return *dev;
	}
}
