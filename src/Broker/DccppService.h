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

#include <vector>

#include "Socket.h"

#include "Service.h"

class DccLiteService;
class DccppClient;
class OutputDecoder;

class DccppService: public Service
{	
	public:
		DccppService(const ServiceClass& serviceClass, const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project);

		void Update(const dcclite::Clock& clock) override;

		void Initialize() override;

	private:
		std::string		m_strDccServiceName;
		DccLiteService *m_pclDccService = nullptr;

		//
		//Network communication
		//
		dcclite::Socket m_clSocket;

		std::vector<DccppClient> m_vecClients;
};