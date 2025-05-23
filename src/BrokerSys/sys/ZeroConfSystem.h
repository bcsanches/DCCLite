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

#include <dcclite/Socket.h>

#include "Service.h"

namespace dcclite::broker::sys::ZeroConfSystem
{ 		
	extern void Start(std::string_view projectName);

	extern void Stop();
		
	//
	// Register a name to be published 
	//
	// It is safe to call this without a call to Start, but publishing service only happens after start is called
	//
	extern void Register(const std::string_view serviceName, const uint16_t port);				
}
