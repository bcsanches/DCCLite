// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "TycoonService.h"

#include "sys/ServiceFactory.h"

#include "FastClock.h"

namespace dcclite::broker::tycoon
{

	class TycoonServiceImpl : public TycoonService
	{
		public:
			TycoonServiceImpl(RName name, sys::Broker &broker, const rapidjson::Value &params);

		private:
			FastClock m_clFastClock;
	};

	TycoonServiceImpl::TycoonServiceImpl(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		TycoonService(name, broker, params),
		m_clFastClock{ RName{"FastClock"}, 4 }
	{
		m_clFastClock.Start();
	}

	//
	//
	// TycoonServiceFactory
	//
	//

	const char *TycoonService::TYPE_NAME = "TycoonService";

	void TycoonService::RegisterFactory()
	{
		//empty
	}

	TycoonService::TycoonService(RName name, sys::Broker &broker, const rapidjson::Value &params) :
		Service(name, broker, params)		
	{
		//empty
	}

	static sys::GenericServiceFactory<TycoonServiceImpl> g_ServiceFactory;
}
