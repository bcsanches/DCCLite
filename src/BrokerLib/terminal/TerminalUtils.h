// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include <string>

#include "TerminalCmd.h"

namespace dcclite::broker
{
	class NetworkDevice;
}

namespace dcclite::broker::detail
{
	constexpr auto JSONRPC_KEY = "jsonrpc";
	constexpr auto JSONRPC_VERSION = "2.0";

	std::string MakeRpcMessage(CmdId_t id, std::string_view *methodName, std::string_view nestedObjName, std::function<void(JsonOutputStream_t &object)> filler);

	inline std::string MakeRpcNotificationMessage(CmdId_t id, std::string_view methodName, std::function<void(JsonOutputStream_t &object)> filler)
	{
		return MakeRpcMessage(id, &methodName, "params", filler);
	}

	inline std::string MakeRpcErrorResponse(const CmdId_t id, const std::string &msg)
	{
		return MakeRpcMessage(id, nullptr, "error", [&, msg](JsonOutputStream_t &params) { params.AddStringValue("message", msg); });
	}

	inline std::string MakeRpcResultMessage(const CmdId_t id, std::function<void(JsonOutputStream_t &object)> filler)
	{
		return MakeRpcMessage(id, nullptr, "result", filler);
	}

	IFolderObject &GetCurrentFolder(const TerminalContext &context, const CmdId_t id);
		
	NetworkDevice &GetNetworkDevice(const dcclite::Path_t &path, const TerminalContext &context, const CmdId_t id);
}
