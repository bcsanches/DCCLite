// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "LocationManagerService.h"

#include <Log.h>

#include "Decoder.h"

using namespace dcclite;

static ServiceClass locationManagerService("LocationManagerService",
	[](const ServiceClass& serviceClass, const std::string& name, Broker& broker, const rapidjson::Value& params, const Project& project) ->
	std::unique_ptr<Service> { return std::make_unique<LocationManagerService>(serviceClass, name, broker, params, project); }
);

class Sector: public Object
{
	public:
		Sector(std::string name, std::string prefix, const DccAddress beginAddress, const DccAddress endAddress): 
			Object(name),
			m_strPrefix(std::move(prefix)),
			m_tBeginAddress(beginAddress),
			m_tEndAddress(endAddress)
		{
			if(m_tBeginAddress > m_tEndAddress)
			{
				throw std::runtime_error(
					fmt::format("[{}] Sector::Sector begin adress ({}) is greater than end address ({})", 
						this->GetName(),
						m_tBeginAddress,
						m_tEndAddress
					)
				);
			}
		}

		const char *GetTypeName() const noexcept override
		{
			return "LocationSector";
		}

		void Serialize(JsonOutputStream_t &stream) const override
		{
			Object::Serialize(stream);

			stream.AddStringValue("prefix", m_strPrefix);
			stream.AddIntValue("begin", m_tBeginAddress.GetAddress());
			stream.AddIntValue("end", m_tEndAddress.GetAddress());
		}

	private:	
		std::string m_strPrefix;
		DccAddress m_tBeginAddress;
		DccAddress m_tEndAddress;
};

LocationManagerService::LocationManagerService(const ServiceClass& serviceClass, const std::string& name, Broker &broker, const rapidjson::Value& params, const Project& project):
	Service(serviceClass, name, broker, params, project)
{
	const rapidjson::Value &sectorsData = params["sectors"];

	if(!sectorsData.IsArray())
		throw new std::runtime_error(fmt::format("[{}] LocationManagerService no sectors data array", this->GetName()));

	for(auto &sectorData : sectorsData.GetArray())
	{
		auto name = sectorData["name"].GetString();
		auto prefix = sectorData["prefix"].GetString();
		auto beginAddress = DccAddress{static_cast<uint16_t>(sectorData["begin"].GetInt())};
		auto endAddress = DccAddress{ static_cast<uint16_t>(sectorData["end"].GetInt())};

		this->AddChild(std::make_unique<Sector>(name, prefix, beginAddress, endAddress));
	}
}

void LocationManagerService::Initialize()
{
	//empty
}

void LocationManagerService::Update(const dcclite::Clock& clock)
{
	//tick tock
}
