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

#include "LogUtils.h"

#include <spdlog/logger.h>

namespace dcclite
{
	namespace Log
	{
		template<typename... Args>
		inline void Trace(const char *fmt, const Args &... args)
		{
			LogGetDefault()->trace(fmt, args...);
		}

		template<typename... Args>
		inline void Debug(const char *fmt, const Args &... args)
		{
			LogGetDefault()->debug(fmt, args...);
		}

		template<typename... Args>
		inline void Info(const char *fmt, const Args &... args)
		{
			LogGetDefault()->info(fmt, args...);
		}

		template<typename... Args>
		inline void Warn(const char *fmt, const Args &... args)
		{
			LogGetDefault()->warn(fmt, args...);
		}

		template<typename... Args>
		inline void Error(const char *fmt, const Args &... args)
		{
			LogGetDefault()->error(fmt, args...);
		}

		template<typename... Args>
		inline void Critical(const char *fmt, const Args &... args)
		{
			LogGetDefault()->critical(fmt, args...);
		}
	}	
}