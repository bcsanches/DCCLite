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

#include <spdlog/spdlog.h>

namespace dcclite
{
	namespace Log
	{
		template<typename... Args>
		inline void Trace(const char *fmt, const Args &... args)
		{
			spdlog::trace(fmt, args...);
		}

		template<typename... Args>
		inline void Debug(const char *fmt, const Args &... args)
		{
			spdlog::debug(fmt, args...);
		}

		template<typename... Args>
		inline void Info(const char *fmt, const Args &... args)
		{
			spdlog::info(fmt, args...);
		}

		template<typename... Args>
		inline void Warn(const char *fmt, const Args &... args)
		{
			spdlog::warn(fmt, args...);
		}

		template<typename... Args>
		inline void Error(const char *fmt, const Args &... args)
		{
			spdlog::error(fmt, args...);
		}

		template<typename... Args>
		inline void Error(const std::string &fmt, const Args &... args)
		{
			spdlog::error(fmt, args...);
		}

		template<typename... Args>
		inline void Critical(const char *fmt, const Args &... args)
		{
			spdlog::critical(fmt, args...);
		}
	} //end of namespace Log
} //end of namespace dcclite