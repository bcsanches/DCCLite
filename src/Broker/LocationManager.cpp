// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.


#include "LocationManager.h"

#include <Log.h>

#include "Decoder.h"

using namespace dcclite;

class Location: public Object
{
	public:
		Location(std::string name, std::string prefix, const DccAddress beginAddress, const DccAddress endAddress):
			Object(name),
			m_strPrefix(std::move(prefix)),
			m_tBeginAddress(beginAddress),
			m_tEndAddress(endAddress)
		{
			if(m_tBeginAddress >= m_tEndAddress)
			{
				throw std::runtime_error(
					fmt::format("[{}] Sector::Sector begin adress ({}) is greater than end address ({})", 
						this->GetName(),
						m_tBeginAddress,
						m_tEndAddress
					)
				);
			}

			m_vecDecoders.resize(m_tEndAddress.GetAddress() - m_tBeginAddress.GetAddress());
		}

		void RegisterDecoder(Decoder &dec)
		{
			assert((dec.GetAddress() >= m_tBeginAddress) && (dec.GetAddress() < m_tEndAddress));

			size_t index = dec.GetAddress().GetAddress() - m_tBeginAddress.GetAddress();

			assert(m_vecDecoders[index] == nullptr);

			m_vecDecoders[index] = &dec;
		}

		void UnregisterDecoder(Decoder &dec)
		{
			size_t index = dec.GetAddress().GetAddress() - m_tBeginAddress.GetAddress();

			assert(m_vecDecoders[index] == &dec);

			m_vecDecoders[index] = nullptr;
		}

		const char *GetTypeName() const noexcept override
		{
			return "Location";
		}

		void Serialize(JsonOutputStream_t &stream) const override
		{
			Object::Serialize(stream);

			stream.AddStringValue("prefix", m_strPrefix);
			stream.AddIntValue("begin", m_tBeginAddress.GetAddress());
			stream.AddIntValue("end", m_tEndAddress.GetAddress());
		}

		inline DccAddress GetBeginAddress() const
		{
			return m_tBeginAddress;
		}

		inline DccAddress GetEndAddress() const
		{
			return m_tEndAddress;
		}

	private:	
		std::string m_strPrefix;
		DccAddress m_tBeginAddress;
		DccAddress m_tEndAddress;

		std::vector<Decoder *> m_vecDecoders;
};

LocationManager::LocationManager(std::string name, const rapidjson::Value& params):
	FolderObject(std::move(name))
{
	auto it = params.FindMember("locations");
	if(it == params.MemberEnd())
		return;

	const rapidjson::Value &sectorsData = it->value;

	if(!sectorsData.IsArray())
		throw new std::runtime_error(fmt::format("[{}] LocationManagerService no sectors data array", this->GetName()));
	
	m_vecIndex.reserve(sectorsData.GetArray().Size());

	for(auto &sectorData : sectorsData.GetArray())
	{
		auto name = sectorData["name"].GetString();
		auto prefix = sectorData["prefix"].GetString();
		auto beginAddress = DccAddress{static_cast<uint16_t>(sectorData["begin"].GetInt())};
		auto endAddress = DccAddress{ static_cast<uint16_t>(sectorData["end"].GetInt())};

		auto location = this->AddChild(std::make_unique<Location>(name, prefix, beginAddress, endAddress));

		m_vecIndex.push_back(static_cast<Location *>(location));
	}

	if(m_vecIndex.empty())
		return;


	//
	//Validates the locations address for overlapping
	std::sort(m_vecIndex.begin(), m_vecIndex.end(), [](Location *lhs, Location *rhs)
	{
		return lhs->GetBeginAddress() < rhs->GetBeginAddress();
	});

	auto *first = m_vecIndex[0];
	for (size_t i = 1, sz = m_vecIndex.size(); i < sz; ++i)
	{
		auto *current = m_vecIndex[i];

		if(first->GetEndAddress() > current->GetBeginAddress())
			throw std::runtime_error(fmt::format("[LocationManager] Location {} overlaps with {}", first->GetName(), current->GetName()));

		first = current;
	}
}

void LocationManager::RegisterDecoder(Decoder &decoder)
{
	auto address = decoder.GetAddress();

	for (auto location : m_vecIndex)
	{
		//not mapped
		if(address < location->GetBeginAddress())
			return;

		if(address >= location->GetEndAddress())
			continue;

		//found it
		location->RegisterDecoder(decoder);
	}
}

void LocationManager::UnregisterDecoder(Decoder &decoder)
{

}

