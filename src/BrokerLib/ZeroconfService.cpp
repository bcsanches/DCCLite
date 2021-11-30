// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "ZeroconfService.h"

#include <magic_enum.hpp>

#include <Log.h>

namespace dcclite::broker
{
	
	class ZeroconfServiceImpl : public ZeroconfService
	{
		public:
			ZeroconfServiceImpl(const std::string &name, Broker &broker, const Project &project);
			~ZeroconfServiceImpl() override;

			void Update(const dcclite::Clock &clock) override;			

			void Serialize(JsonOutputStream_t &stream) const override;		

			void Register(const Service &service, const uint16_t port) override;

		private:			
	};


	ZeroconfServiceImpl::ZeroconfServiceImpl(const std::string& name, Broker &broker, const Project& project):
		ZeroconfService(name, broker, project)
	{				
		
	}
	

	ZeroconfServiceImpl::~ZeroconfServiceImpl()
	{
		//empty
	}


	void ZeroconfServiceImpl::Update(const dcclite::Clock& clock)
	{	
									
	}

	void ZeroconfServiceImpl::Serialize(JsonOutputStream_t &stream) const
	{
		ZeroconfService::Serialize(stream);
	}

	void ZeroconfServiceImpl::Register(const Service &service, const uint16_t port)
	{

	}

	
	///////////////////////////////////////////////////////////////////////////////
	//
	// ZeroconfService
	//
	///////////////////////////////////////////////////////////////////////////////

	ZeroconfService::ZeroconfService(const std::string &name, Broker &broker, const Project &project) :
		Service(name, broker, project)
	{
		//empty
	}

	std::unique_ptr<Service> ZeroconfService::Create(const std::string &name, Broker &broker, const Project &project)
	{
		return std::make_unique<ZeroconfServiceImpl>(name, broker, project);
	}
}
