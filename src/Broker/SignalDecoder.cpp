// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "SignalDecoder.h"

static Decoder::Class signalDecoderClass("VirtualSignal",
	[](const Decoder::Class &decoderClass, const DccAddress &address, const std::string &name, IDccLite_DecoderServices &owner, IDevice_DecoderServices &dev, const rapidjson::Value &params)
	-> std::unique_ptr<Decoder> { return std::make_unique<SignalDecoder>(decoderClass, address, name, owner, dev, params); }
);


SignalDecoder::SignalDecoder(
	const Class &decoderClass,
	const DccAddress &address,
	const std::string &name,
	IDccLite_DecoderServices &owner,
	IDevice_DecoderServices &dev,
	const rapidjson::Value &params
) :
	Decoder(decoderClass, address, name, owner, dev, params)
{
	auto headsData = params.FindMember("heads");
	if ((headsData == params.MemberEnd()) || (!headsData->value.IsObject()))
	{
		throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: expected heads object for {}", this->GetName()));		
	}

	for (auto &headElement : headsData->value.GetObject())
	{
		m_mapHeads.insert(std::make_pair(headElement.name.GetString(), headElement.value.GetString()));
	}

	auto aspectsData = params.FindMember("aspects");
	if ((aspectsData == params.MemberEnd()) || (!aspectsData->value.IsArray()))
	{
		throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: expected aspects array for {}", this->GetName()));
	}

	for (auto &aspectElement : aspectsData->value.GetArray())
	{
		auto name = aspectElement["name"].GetString();

		auto aspectId = dcclite::ConvertNameToAspect(name);

		if(std::any_of(m_vecAspects.begin(), m_vecAspects.end(), [aspectId](const Aspect &aspect) { return aspect.m_eAspect == aspectId; }))		
		{
			throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: aspect {} already defined", name));
		}

		Aspect newAspect;

		newAspect.m_eAspect = aspectId;

		auto onLights = aspectElement.FindMember("on");
		if (onLights != aspectElement.MemberEnd())
		{
			for (auto &it : onLights->value.GetArray())
			{
				if (m_mapHeads.find(it.GetString()) == m_mapHeads.end())
				{
					throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: aspect {} on \"on\" array not found on heads defintion", it.GetString()));
				}

				newAspect.m_vecOnHeads.push_back(it.GetString());
			}
		}

		auto offLights = aspectElement.FindMember("off");
		if (offLights != aspectElement.MemberEnd())
		{
			for (auto &it : offLights->value.GetArray())
			{
				if (m_mapHeads.find(it.GetString()) == m_mapHeads.end())
				{
					throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: aspect {} on \"off\" array not found on heads defintion", it.GetString()));
				}

				if (std::any_of(newAspect.m_vecOnHeads.begin(), newAspect.m_vecOnHeads.end(), [&it](const std::string &onAspectName) { return onAspectName.compare(it.GetString()) == 0; }))
				{
					throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: \"off\" head {} also defined on the \"on\" table", it.GetString()));
				}

				newAspect.m_vecOffHeads.push_back(it.GetString());
			}
		}
		else
		{
			//if no off array defined, we simple add all heads not listed on the "on" table
			for (auto headIt : m_mapHeads)
			{
				//skip existing heads
				if (std::any_of(newAspect.m_vecOnHeads.begin(), newAspect.m_vecOnHeads.end(), [headIt](const std::string &onAspectName) { return onAspectName.compare(headIt.first) == 0; }))
				{
					continue;
				}

				newAspect.m_vecOffHeads.push_back(headIt.first);
			}
		}

		m_vecAspects.push_back(std::move(newAspect));
	}

	std::sort(m_vecAspects.begin(), m_vecAspects.end(), [](const Aspect &a, const Aspect &b)
	{
		return a.m_eAspect < b.m_eAspect;
	});
}


void SignalDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
{
	Decoder::Serialize(stream);



}

