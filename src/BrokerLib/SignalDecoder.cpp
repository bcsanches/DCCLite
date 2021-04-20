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

#include <Log.h>

#include "IDccLiteService.h"
#include "OutputDecoder.h"

// ms define leak...
#ifdef GetObject
#undef GetObject
#endif

namespace dcclite::broker
{
	using namespace std::chrono_literals;
	static auto constexpr FLASH_INTERVAL = 300ms;


	SignalDecoder::SignalDecoder(
		const DccAddress &address,
		const std::string &name,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		Decoder(address, name, owner, dev, params)
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
		if ((aspectsData == params.MemberEnd()) || (!aspectsData->value.IsArray()) || (aspectsData->value.GetArray().Empty()))
		{
			throw std::invalid_argument(fmt::format("[SignalDecoder::SignalDecoder] Error: expected aspects array for {}", this->GetName()));
		}

		for (auto &aspectElement : aspectsData->value.GetArray())
		{
			auto name = aspectElement["name"].GetString();

			auto aspectId = dcclite::ConvertNameToAspect(name);

			if (std::any_of(m_vecAspects.begin(), m_vecAspects.end(), [aspectId](const Aspect &aspect) { return aspect.m_eAspect == aspectId; }))
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
				return b.m_eAspect < a.m_eAspect;
			});
	}


	void SignalDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Decoder::Serialize(stream);



	}

	void SignalDecoder::ForEachHead(const std::vector<std::string> &heads, const dcclite::SignalAspects aspect, std::function<bool(OutputDecoder &)> proc) const
	{
		for (auto head : heads)
		{
			auto *dec = dynamic_cast<OutputDecoder *>(m_rclManager.TryFindDecoder(head));
			if (dec == nullptr)
			{
				dcclite::Log::Error("[SignalDecoder::SetAspect] Head {} for aspect {} not found or is not an output decoder.", head, dcclite::ConvertAspectToName(aspect));
				continue;
			}

			if (!proc(*dec))
				break;
		}
	}

	void SignalDecoder::SetAspect(const dcclite::SignalAspects aspect, const char *requester)
	{
		if (aspect == m_eCurrentAspect)
			return;

		int index = 0;
		for (auto it : m_vecAspects)
		{
			if (it.m_eAspect <= aspect)
			{
				break;
			}

			++index;
		}

		if (index == m_vecAspects.size())
			--index;		

		this->ForEachHead(m_vecAspects[index].m_vecOffHeads, aspect, [this](OutputDecoder &dec)
			{
				dec.SetState(dcclite::DecoderStates::INACTIVE, this->GetName().data());

				return true;
			}
		);

		m_eCurrentAspect = aspect;
		m_uCurrentAspectIndex = index;

		m_vState = State_WaitTurnOff{};		
		m_pclCurrentState = std::get_if<State_WaitTurnOff>(&m_vState);
	}

	void SignalDecoder::Update(const dcclite::Clock &clock)
	{
		if (m_pclCurrentState)
			m_pclCurrentState->Update(*this, clock.Now());
	}
	
	void SignalDecoder::State_WaitTurnOff::Update(SignalDecoder &self, const dcclite::Clock::TimePoint_t time)
	{
		bool stillPending = false;
		self.ForEachHead(self.m_vecAspects[self.m_uCurrentAspectIndex].m_vecOffHeads, self.m_eCurrentAspect, [&stillPending](OutputDecoder &dec)
			{
				if (dec.GetRemoteState() == dcclite::DecoderStates::INACTIVE)
				{
					stillPending = true;

					return false;
				}

				return true;
			}
		);

		if (stillPending)
			return;

		self.ForEachHead(self.m_vecAspects[self.m_uCurrentAspectIndex].m_vecOnHeads, self.m_eCurrentAspect, [&self](OutputDecoder &dec)
			{
				dec.Activate(self.GetName().data());

				return true;
			}
		);

		if (self.m_vecAspects[self.m_uCurrentAspectIndex].m_Flash)
		{
			self.m_vState = State_Flash{time};
			self.m_pclCurrentState = std::get_if<State_Flash>(&self.m_vState);
		}
		else
		{
			self.m_vState = NullState{};
			self.m_pclCurrentState = nullptr;
		}		
	}
	
	SignalDecoder::State_Flash::State_Flash(const dcclite::Clock::TimePoint_t time):
		m_tNextThink(time + FLASH_INTERVAL)
	{
		//empty
	}
	
	void SignalDecoder::State_Flash::Update(SignalDecoder &self, const dcclite::Clock::TimePoint_t time)
	{
		if (time < m_tNextThink)
			return;

		m_fOn = !m_fOn;

		const auto state = m_fOn ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE;

		self.ForEachHead(self.m_vecAspects[self.m_uCurrentAspectIndex].m_vecOnHeads, self.m_eCurrentAspect, [state, &self](OutputDecoder &dec)
			{
				dec.SetState(state, self.GetName().data());

				return true;
			}
		);

		m_tNextThink = time + FLASH_INTERVAL;
	}		
}