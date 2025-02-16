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
		inline void Trace(spdlog::format_string_t<Args...> fmt, Args &&...args)
		{
			spdlog::trace(fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void Debug(spdlog::format_string_t<Args...> fmt, Args &&...args)
		{
			spdlog::debug(fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void Info(spdlog::format_string_t<Args...> fmt, Args &&...args)
		{
			spdlog::info(fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void Warn(spdlog::format_string_t<Args...> fmt, Args &&...args)
		{
			spdlog::warn(fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void Error(spdlog::format_string_t<Args...> fmt, Args &&...args)
		{
			spdlog::error(fmt, std::forward<Args>(args)...);
		}

		template<typename... Args>
		inline void Critical(spdlog::format_string_t<Args...> fmt, Args &&...args)
		{
			spdlog::critical(fmt, std::forward<Args>(args)...);
		}

		typedef std::shared_ptr<spdlog::logger> Logger_t;

		namespace detail
		{
			extern void Init(const char *fileName);
			extern void Finalize();
		}

		extern void Replace(Logger_t log);

		extern Logger_t GetDefault();

	} //end of namespace Log
} //end of namespace dcclite