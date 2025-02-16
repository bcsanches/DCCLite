// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Guid.h"

#include <Windows.h>

#include <fmt/format.h>

#include <dcclite_shared/Misc.h>

#include "FmtUtils.h"

#include "Util.h"

dcclite::Guid dcclite::GuidCreate()
{
	GUID g;

	auto h = CoCreateGuid(&g);
	if (h != S_OK)
	{
		throw std::runtime_error(fmt::format("[GUID::Create] CoCreateGuid failed: {}", h));
	}

	dcclite::Guid guid;

	static_assert(sizeof(g.Data1) == 4);
	static_assert(sizeof(g.Data2) == 2);
	static_assert(sizeof(g.Data3) == 2);
	static_assert(sizeof(g.Data4) == 8);

	memcpy(&guid.m_bId[0], &g.Data1, 4);
	memcpy(&guid.m_bId[4], &g.Data2, 2);
	memcpy(&guid.m_bId[6], &g.Data3, 2);
	memcpy(&guid.m_bId[8], &g.Data4, 8);

#if 0
	dcclite::Log::Debug("{}", guid);

	dcclite::Log::Debug("{}", memcmp(&guid, &g, sizeof(guid)));
#endif

	return guid;
}

