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

#include <FmtUtils.h>

#include <JsonUtils.h>
#include <Log.h>

#include "Decoder.h"

namespace dcclite::broker
{
	namespace detail
	{
		Location::Location(RName name, RName prefix, const DccAddress beginAddress, const DccAddress endAddress, LocationManager &owner) :
			IObject{ name },
			m_rnPrefix(prefix),
			m_tBeginAddress(beginAddress),
			m_tEndAddress(endAddress),
			m_rclParent{&owner}
		{
			if (m_tBeginAddress >= m_tEndAddress)
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

		void Location::RegisterDecoder(const Decoder &dec)
		{
			auto index = GetDecoderIndex(dec);

			assert(m_vecDecoders[index] == nullptr);

			m_vecDecoders[index] = &dec;
		}

		void Location::UnregisterDecoder(const Decoder &dec)
		{
			auto index = GetDecoderIndex(dec);

			m_vecDecoders[index] = nullptr;
		}

		void Location::Serialize(JsonOutputStream_t &stream) const
		{
			IObject::Serialize(stream);

			stream.AddStringValue("prefix", m_rnPrefix.GetData());
			stream.AddIntValue("begin", m_tBeginAddress.GetAddress());
			stream.AddIntValue("end", m_tEndAddress.GetAddress());

			if (m_vecDecoders.empty())
				return;

			auto outputArray = stream.AddArray("decodersPath");

			for(size_t i = 0, sz = m_vecDecoders.size(); i < sz; ++i)
			{
				auto dec = m_vecDecoders[i];
				if (!dec)
					continue;

				auto obj = outputArray.AddObject();
				obj.AddIntValue("index", (int)i);
				obj.AddStringValue("path", dec->GetPath().string());
			}
		}

		inline size_t Location::GetDecoderIndex(const Decoder &dec) const
		{
			assert((dec.GetAddress() >= m_tBeginAddress) && (dec.GetAddress() < m_tEndAddress));

			return dec.GetAddress().GetAddress() - m_tBeginAddress.GetAddress();
		}

		IFolderObject *Location::GetParent() const noexcept
		{
			return m_rclParent;
		}
	}

	IObject *LocationManager::TryGetChild(RName name)
	{
		for (auto &location : m_vecIndex)
		{
			if (location.GetName() == name)
				return &location;
		}

		return nullptr;
	}

	void LocationManager::VisitChildren(Visitor_t visitor)
	{
		for (auto &location : m_vecIndex)
		{
			visitor(location);
		}
	}

	LocationManager::LocationManager(RName name, const rapidjson::Value& params):
		IFolderObject{name}
	{
		auto it = params.FindMember("locations");
		if(it == params.MemberEnd())
			return;

		const rapidjson::Value &sectorsData = it->value;

		if(!sectorsData.IsArray())
			throw new std::runtime_error(fmt::format("[{}] LocationManagerService sectors data must be an array", this->GetName()));
	
		m_vecIndex.reserve(sectorsData.GetArray().Size());

		for(auto &sectorData : sectorsData.GetArray())
		{
			RName dname{ json::GetString(sectorData, "name", "LocationManager::Sector")};
			RName prefix{ json::GetString(sectorData, "prefix", "LocationManager::Sector") };
			auto beginAddress = DccAddress{static_cast<uint16_t>(json::GetInt(sectorData, "begin", "LocationManager::Sector"))};
			auto endAddress = DccAddress{ static_cast<uint16_t>(json::GetInt(sectorData, "end", "LocationManager::Sector"))};

			m_vecIndex.emplace_back(dname, prefix, beginAddress, endAddress, *this);

			detail::Location a{ dname, prefix, beginAddress, endAddress, *this };
			detail::Location b{ dname, prefix, beginAddress, endAddress, *this };
		}

		if(m_vecIndex.empty())
			return;

#if 1
		//
		//Validates the locations address for overlapping
		std::sort(m_vecIndex.begin(), m_vecIndex.end(), [](detail::Location &lhs, detail::Location &rhs)
		{
			return lhs.GetBeginAddress() < rhs.GetBeginAddress();
		});
#endif

		auto first = &m_vecIndex[0];
		for (size_t i = 1, sz = m_vecIndex.size(); i < sz; ++i)
		{
			auto *current = &m_vecIndex[i];

			if(first->GetEndAddress() > current->GetBeginAddress())
				throw std::runtime_error(fmt::format("[LocationManager] Location {} overlaps with {}", first->GetName(), current->GetName()));

			first = current;
		}
	}

	void LocationManager::RegisterDecoder(const Decoder &decoder)
	{
		auto address = decoder.GetAddress();

		for (auto &location : m_vecIndex)
		{
			//not mapped?
			if(address < location.GetBeginAddress())
				break;

			if(address >= location.GetEndAddress())
				continue;

			//found it
			auto locationHint = decoder.GetLocationHint();
			if ((locationHint) && (locationHint != location.GetPrefix()) && (locationHint != location.GetName()))
			{			
				//loaction hint does not match
				m_vecMismatches.emplace_back(				
					&decoder, 
					LocationMismatchReason::WRONG_LOCATION_HINT,
					&location				
				);

				return;
			}

			location.RegisterDecoder(decoder);
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

		for (auto &location : m_vecIndex)
		{
			//not mapped
			if (address < location.GetBeginAddress())
				break;

			if (address >= location.GetEndAddress())
				continue;

			if(location.IsDecoderRegistered(decoder))
			{
				location.UnregisterDecoder(decoder);

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
		IFolderObject::Serialize(stream);

		if(m_vecMismatches.empty())
			return;

		auto mismatchesArray = stream.AddArray("mismatches");
		for (auto tuple : m_vecMismatches)
		{
			auto obj = mismatchesArray.AddObject();
			obj.AddStringValue("reason", LocationMismatchReason2String(std::get<1>(tuple)));
			obj.AddStringValue("decoderPath", std::get<0>(tuple)->GetPath().string());
					
			auto location = std::get<2>(tuple);
			if(location != nullptr)
				obj.AddStringValue("location", location->GetName().GetData());
		}
	}

}