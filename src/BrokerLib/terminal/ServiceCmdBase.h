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

#include "TerminalCmd.h"

#include <fmt/format.h>

#include "../sys/SpecialFolders.h"

namespace dcclite::broker
{ 
	class DccLiteService;

	class DccLiteCmdBase: public TerminalCmd
	{
		protected:
			explicit DccLiteCmdBase(RName name):
				TerminalCmd(name)
			{
				//empty
			}

			template <typename T>
			T &GetService(const TerminalContext &context, const CmdId_t id, std::string_view serviceName)
			{
				auto &root = static_cast<FolderObject &>(context.TryGetItem()->GetRoot());

				ObjectPath path{ SpecialFolders::GetPath(SpecialFolders::Folders::ServicesId) };
				path.append(serviceName);

				auto obj = root.TryNavigate(path);
				if (obj == nullptr)
				{
					throw TerminalCmdException(fmt::format("Service {} not found", serviceName), id);
				}

				auto *service = dynamic_cast<T *>(obj);
				if (service == nullptr)
				{
					throw TerminalCmdException(fmt::format("Service {} does not has requested type", serviceName, typeid(T).name()), id);
				}

				return *service;
			}

			DccLiteService &GetDccLiteService(const TerminalContext &context, const CmdId_t id, std::string_view dccSystemName);
	};
}
