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

namespace dcclite::broker
{

	class Location: public Object
	{
		public:
			Location(std::string name, std::string prefix, const DccAddress beginAddress, const DccAddress endAddress):
				Object(std::move(name)),
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

			void RegisterDecoder(const Decoder &dec)
			{
				auto index = GetDecoderIndex(dec);

				assert(m_vecDecoders[index] == nullptr);

				m_vecDecoders[index] = &dec;
			}

			void UnregisterDecoder(const Decoder &dec)
			{
				auto index = GetDecoderIndex(dec);

				m_vecDecoders[index] = nullptr;
			}

			bool IsDecoderRegistered(const Decoder &dec) const
			{
				return m_vecDecoders[GetDecoderIndex(dec)] == &dec;
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

				if(m_vecDecoders.empty())
					return;

				auto outputArray = stream.AddArray("decoders");

				for (auto &dec : m_vecDecoders)
				{
					if (dec == nullptr)
					{
						outputArray.AddNull();
					}
					else
					{
						auto obj = outputArray.AddObject();
						dec->Serialize(obj);
					}				
				}
			}

			inline DccAddress GetBeginAddress() const
			{
				return m_tBeginAddress;
			}

			inline DccAddress GetEndAddress() const
			{
				return m_tEndAddress;
			}

			inline const std::string &GetPrefix() const
			{
				return m_strPrefix;
			}

		private:
			inline size_t GetDecoderIndex(const Decoder &dec) const
			{
				assert((dec.GetAddress() >= m_tBeginAddress) && (dec.GetAddress() < m_tEndAddress));

				return dec.GetAddress().GetAddress() - m_tBeginAddress.GetAddress();
			}

		private:	
			std::string m_strPrefix;
			DccAddress m_tBeginAddress;
			DccAddress m_tEndAddress;

			std::vector<const Decoder *> m_vecDecoders;
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
			auto dname = sectorData["name"].GetString();
			auto prefix = sectorData["prefix"].GetString();
			auto beginAddress = DccAddress{static_cast<uint16_t>(sectorData["begin"].GetInt())};
			auto endAddress = DccAddress{ static_cast<uint16_t>(sectorData["end"].GetInt())};

			auto location = this->AddChild(std::make_unique<Location>(dname, prefix, beginAddress, endAddress));

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

	void LocationManager::RegisterDecoder(const Decoder &decoder)
	{
		auto address = decoder.GetAddress();

		for (auto location : m_vecIndex)
		{
			//not mapped?
			if(address < location->GetBeginAddress())
				break;

			if(address >= location->GetEndAddress())
				continue;

			//found it
			auto &locationHint = decoder.GetLocationHint();
			if ((!locationHint.empty()) && (locationHint.compare(location->GetPrefix())) && (locationHint.compare(location->GetName())))
			{			
				//loaction hint does not match
				m_vecMismatches.emplace_back(				
					&decoder, 
					LocationMismatchReason::WRONG_LOCATION_HINT,
					location				
				);

				return;
			}

			location->RegisterDecoder(decoder);
			return;
		}

		m_vecMismatches.emplace_back(
			&decoder, 
			LocationMismatchReason::OUTSIDE_RANGES,
			nullptr
		);
	}

	void LocationManager::UnregisterDecoder(const Decoder &decoder)
	{
		auto address = decoder.GetAddress();

		for (auto location : m_vecIndex)
		{
			//not mapped
			if (address < location->GetBeginAddress())
				break;

			if (address >= location->GetEndAddress())
				continue;

			if(location->IsDecoderRegistered(decoder))
			{
				location->UnregisterDecoder(decoder);

				return;
			}

			//not mapped
			break;
		}

		//It should be on the mismatches list
		auto it = std::find_if(m_vecMismatches.begin(), m_vecMismatches.end(), [&decoder](const auto &tuple) {
			return std::get<0>(tuple) == &decoder;
		});

		if(it == m_vecMismatches.end())
		{
			//Ouch! Something bad happened
			throw std::runtime_error(fmt::format("[LocationManager::UnregisterDecoder] Decoder {} - {} not found anywhere!!", decoder.GetName(), decoder.GetAddress().GetAddress()));
		}
		else
		{
			//remove it
			m_vecMismatches.erase(it);
		}	
	}

	static const char *LocationMismatchReason2String(LocationMismatchReason reason)
	{
		switch (reason)
		{
			case LocationMismatchReason::OUTSIDE_RANGES:
				return "outside_range";

			case LocationMismatchReason::WRONG_LOCATION_HINT:
				return "wrong_hint";

			default:
				assert(0);

				//die! die!
				return nullptr;
		}
	}

	void LocationManager::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		FolderObject::Serialize(stream);

		if(m_vecMismatches.empty())
			return;

		auto mismatchesArray = stream.AddArray("mismatches");
		for (auto tuple : m_vecMismatches)
		{
			auto obj = mismatchesArray.AddObject();
			obj.AddStringValue("reason", LocationMismatchReason2String(std::get<1>(tuple)));
		
			auto decoderObj = obj.AddObject("decoder");
			std::get<0>(tuple)->Serialize(decoderObj);

			auto location = std::get<2>(tuple);
			if(location != nullptr)
				obj.AddStringValue("location", location->GetName());
		}
	}

}