// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "BonjourService.h"

#include <Log.h>

#include "Socket.h"

namespace dcclite::broker
{
	///////////////////////////////////////////////////////////////////////////////
	//
	// ThrottleServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	class BonjourServiceImpl : public BonjourService
	{
		public:
			BonjourServiceImpl(const std::string &name, Broker &broker, const Project &project);
			~BonjourServiceImpl() override;

			void Update(const dcclite::Clock &clock) override;			

			void Serialize(JsonOutputStream_t &stream) const override;				

		private:		
			dcclite::Socket m_clSocket;
	};


	BonjourServiceImpl::BonjourServiceImpl(const std::string& name, Broker &broker, const Project& project):
		BonjourService(name, broker, project)
	{				
		if (!m_clSocket.Open(5353, dcclite::Socket::Type::DATAGRAM, dcclite::Socket::FLAG_ADDRESS_REUSE))
		{
			throw std::runtime_error("[BonjourServiceImpl] Cannot open port 5353 for listening");
		}
	}
	

	BonjourServiceImpl::~BonjourServiceImpl()
	{
		//empty
	}

	void BonjourServiceImpl::Update(const dcclite::Clock& clock)
	{	
		uint8_t cache[128];

		auto [status, size] = m_clSocket.Receive(cache, sizeof(cache));

		if (status != dcclite::Socket::Status::OK)
			return;

		dcclite::Log::Trace(" got something");
	}

	void BonjourServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		BonjourService::Serialize(stream);
	}
	
	///////////////////////////////////////////////////////////////////////////////
	//
	// ThrottleServiceImpl
	//
	///////////////////////////////////////////////////////////////////////////////

	BonjourService::BonjourService(const std::string &name, Broker &broker, const Project &project) :
		Service(name, broker, project)
	{
		//empty
	}

	std::unique_ptr<Service> BonjourService::Create(const std::string &name, Broker &broker, const Project &project)
	{
		return std::make_unique<BonjourServiceImpl>(name, broker, project);
	}
}