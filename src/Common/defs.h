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

#ifdef _WIN64
#define DCCLITE64
#elif defined _WIN32
#define DCCLITE32
#elif defined __linux__
	#if defined __x86_64__
		#define DCCLITE64	
	#else
		#define DCCLITE32
	#endif
#endif

#if (!defined DCCLITE64) && (!defined DCCLITE32)
#error "plataform not defined"
#endif
