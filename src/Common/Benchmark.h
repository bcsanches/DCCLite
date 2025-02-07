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

#include <chrono>
#include <string_view>

namespace dcclite
{
	class Benchmark
	{
		public:
			typedef long double us_t;
			typedef long double ms_t;
			typedef long double second_t;

		public:
			Benchmark();

			Benchmark(const Benchmark &rhs) = delete;
			Benchmark(Benchmark &&rhs) = delete;

			void Start();

			void Stop();

			inline us_t GetUs() const noexcept
			{
				return (us_t)std::chrono::duration_cast<std::chrono::microseconds>(m_tEnd - m_tStart).count();
			}

			inline ms_t GetMs() const noexcept
			{
				return this->GetUs() / 1000.0;
			}

			inline second_t GetSecond() const noexcept
			{
				return this->GetMs() / 1000.0;
			}

		private:
			typedef std::chrono::time_point<std::chrono::high_resolution_clock> TimePoint_t;

			TimePoint_t m_tStart;
			TimePoint_t m_tEnd;

	};

	class BenchmarkLogger
	{
		public:
			inline BenchmarkLogger(std::string_view moduleName, std::string_view message):
				m_svModuleName(moduleName),
				m_svMessage(message)
			{
				//empty
			}

			~BenchmarkLogger();

		private:
			Benchmark m_clBenchmark;

			std::string_view m_svModuleName;
			std::string_view m_svMessage;
	};
}
