// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Benchmark.h"

#include "Log.h"

namespace dcclite
{
	Benchmark::Benchmark()
	{
		this->Start();
	}

	void Benchmark::Start()
	{
		m_tEnd = m_tStart = std::chrono::high_resolution_clock::now();
	}

	void Benchmark::Stop()
	{
		m_tEnd = std::chrono::high_resolution_clock::now();
	}

	BenchmarkLogger::~BenchmarkLogger()
	{
		m_clBenchmark.Stop();

		dcclite::Log::Info("[{}] [Benchmark] {} took: {}ms", m_svModuleName, m_svMessage, m_clBenchmark.GetMs());
	}
}
