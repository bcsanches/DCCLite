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
}
