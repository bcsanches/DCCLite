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

#include "FmtUtils.h"

#include "IDccLiteService.h"
#include "OutputDecoder.h"

// ms define leak...
#ifdef GetObject
#undef GetObject
#endif

namespace dcclite::broker
{
	using namespace std::chrono_literals;
	static auto constexpr FLASH_INTERVAL = 500ms;
	static auto constexpr WAIT_STATE_TIMEOUT = 250ms;


	SignalDecoder::SignalDecoder(
		const DccAddress &address,
		RName decoderName,
		IDccLite_DecoderServices &owner,
		IDevice_DecoderServices &dev,
		const rapidjson::Value &params
	) :
		Decoder(address, decoderName, owner, dev, params)
	{
		auto headsData = params.FindMember("heads");
		if (headsData == params.MemberEnd())
		{
			throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: expected heads object", this->GetName()));
		}

		if (!headsData->value.IsObject())
		{
			throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: heads data is not an object", this->GetName()));
		}

		for (auto &headElement : headsData->value.GetObject())
		{
			m_mapHeads.insert(std::make_pair(headElement.name.GetString(), headElement.value.GetString()));
		}

		auto aspectsData = params.FindMember("aspects");
		if (aspectsData == params.MemberEnd())
		{
			throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: expected aspects array", this->GetName()));
		}

		if (!aspectsData->value.IsArray())
		{
			throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: expected aspects data to be an array", this->GetName()));
		}

		if (aspectsData->value.GetArray().Empty())
		{
			throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: aspects array is empty", this->GetName()));
		}

		for (auto &aspectElement : aspectsData->value.GetArray())
		{
			auto name = aspectElement["name"].GetString();

			auto aspectId = dcclite::ConvertNameToAspect(name);

			if (std::any_of(m_vecAspects.begin(), m_vecAspects.end(), [aspectId](const Aspect &aspect) { return aspect.m_kAspect == aspectId; }))
			{
				throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: aspect {} already defined", this->GetName(), name));
			}

			Aspect newAspect;

			newAspect.m_kAspect = aspectId;

			auto onLights = aspectElement.FindMember("on");
			if (onLights != aspectElement.MemberEnd())
			{
				for (auto &it : onLights->value.GetArray())
				{
					auto headIt = m_mapHeads.find(RName{ it.GetString() });
					if (headIt == m_mapHeads.end())
					{
						throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: aspect {} on \"on\" array not found on heads defintion", this->GetName(), it.GetString()));
					}

					newAspect.m_vecOnHeads.push_back(RName{ headIt->second });
				}
			}

			auto offLights = aspectElement.FindMember("off");
			if (offLights != aspectElement.MemberEnd())
			{
				for (auto &it : offLights->value.GetArray())
				{
					auto headIt = m_mapHeads.find(RName{ it.GetString() });
					if (headIt == m_mapHeads.end())
					{
						throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: aspect {} on \"off\" array not found on heads defintion", this->GetName(), it.GetString()));
					}

					RName headName{ headIt->second };

					if (std::any_of(newAspect.m_vecOnHeads.begin(), newAspect.m_vecOnHeads.end(), [headName](RName onAspectName) { return onAspectName == headName; }))
					{
						throw std::invalid_argument(fmt::format("[SignalDecoder::{}] [SignalDecoder] Error: \"off\" head {} also defined on the \"on\" table", this->GetName(), it.GetString()));
					}

					newAspect.m_vecOffHeads.push_back(headName);
				}
			}
			else
			{
				//if no off array defined, we simple add all heads not listed on the "on" table
				for (auto &headIt : m_mapHeads)
				{
					RName headName{ headIt.second };

					//skip existing heads
					if (std::any_of(newAspect.m_vecOnHeads.begin(), newAspect.m_vecOnHeads.end(), [headName](RName onAspectName) { return onAspectName == headName; }))
					{
						continue;
					}

					newAspect.m_vecOffHeads.push_back(headName);
				}
			}

			auto flashData = aspectElement.FindMember("flash");
			newAspect.m_Flash = (flashData != aspectElement.MemberEnd()) ? flashData->value.GetBool() : false;
			

			m_vecAspects.push_back(std::move(newAspect));
		}		

		std::sort(m_vecAspects.begin(), m_vecAspects.end(), [](const Aspect &a, const Aspect &b)
		{
			return b.m_kAspect < a.m_kAspect;
		});

		//start with most restrictive aspect, expected to be Stop
		const auto aspectIndex = static_cast<unsigned>(m_vecAspects.size() - 1);
		this->ApplyAspect(m_vecAspects[aspectIndex].m_kAspect, aspectIndex);		
	}


	void SignalDecoder::Serialize(dcclite::JsonOutputStream_t &stream) const
	{
		Decoder::Serialize(stream);

		stream.AddStringValue("requestedAspectName", dcclite::ConvertAspectToName(m_eCurrentAspect));
		stream.AddStringValue("currentAspectName", dcclite::ConvertAspectToName(m_vecAspects[m_uCurrentAspectIndex].m_kAspect));
		auto aspectsData = stream.AddArray("aspects");
		
		for (const auto &item : m_vecAspects)
			aspectsData.AddString(dcclite::ConvertAspectToName(item.m_kAspect));
	}

	void SignalDecoder::ForEachHead(const std::vector<RName> &heads, const dcclite::SignalAspects aspect, std::function<bool(OutputDecoder &)> proc) const
	{
		for (const auto &head : heads)
		{			
			auto *dec = dynamic_cast<OutputDecoder *>(m_rclManager.TryFindDecoder(head));
			if (dec == nullptr)
			{
				dcclite::Log::Error("[SignalDecoder::{}] [SetAspect] Head {} for aspect {} not found or is not an output decoder.", this->GetName(), head, dcclite::ConvertAspectToName(aspect));
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
		for (const auto &it : m_vecAspects)
		{
			if (it.m_kAspect <= aspect)
			{
				break;
			}

			++index;
		}

		if (index == m_vecAspects.size())
			--index;		

		//warn user...
		if (aspect != m_vecAspects[index].m_kAspect)
		{
			Log::Warn(
				"[SignalDecoder::{}] [SetAspect] Aspect {} requested by {} not found, using {}", 
				this->GetName(), 
				dcclite::ConvertAspectToName(aspect), 
				requester,
				dcclite::ConvertAspectToName(m_vecAspects[index].m_kAspect)
			);
		}
		
		Log::Info(
			"[SignalDecoder::{}] [SetAspect] Changed from {} to {}, requested by {}",
			this->GetName(),
			dcclite::ConvertAspectToName(m_vecAspects[m_uCurrentAspectIndex].m_kAspect),
			dcclite::ConvertAspectToName(aspect),
			requester
		);

		this->ApplyAspect(aspect, index);
			
		m_rclManager.Decoder_OnStateChanged(*this);
	}

	void SignalDecoder::ApplyAspect(const dcclite::SignalAspects aspect, const unsigned aspectIndex)
	{
		m_eCurrentAspect = aspect;
		m_uCurrentAspectIndex = aspectIndex;

		//
		//We have a turnoff state so on the first init it to turn off all heads...
		m_vState.emplace<State_WaitTurnOff>(*this);	
		auto &waitTurnOffState = std::get<State_WaitTurnOff>(m_vState);		

		//No heads changed to off state?
		if (waitTurnOffState.GetWaitListSize() == 0)
		{
			//Ok, so turn on whatever needs t be 
			waitTurnOffState.GotoNextState();
		}		
		//else
		//wait for decoders events on State_WaitTurnOff
	}	

	SignalDecoder::State_WaitTurnOff::State_WaitTurnOff(SignalDecoder &self):
		State{self},
		m_clTimeoutThinker{ "SignalDecoder::State_WaitTurnOff::TimeoutThinker", THINKER_MF_LAMBDA(OnThink)}
	{
		this->Init();

		//do we have to wait for any heads to turn off?
		if(m_uWaitListSize)
			m_clTimeoutThinker.Schedule(dcclite::Clock::DefaultClock_t::now() + WAIT_STATE_TIMEOUT);
	}

	void SignalDecoder::State_WaitTurnOff::Init()
	{
		m_rclOwner.ForEachHead(m_rclOwner.m_vecAspects[m_rclOwner.m_uCurrentAspectIndex].m_vecOffHeads, m_rclOwner.m_eCurrentAspect, [this](OutputDecoder &dec)
			{
				if (dec.SetState(dcclite::DecoderStates::INACTIVE, m_rclOwner.GetName().GetData().data()))
				{
					m_lstConnections.emplace_back<sigslot::scoped_connection>(
						dec.m_sigRemoteStateSync.connect(&State_WaitTurnOff::OnDecoderStateSync, this)
					);

					m_uWaitListSize++;
				}

				return true;
			}
		);
	}


	/**
	*	We have a simple timeout mechanism to cover some extreme cases, like a device disconnecting while we are waiting 
	* 
	*	It does not seems to be worth managing all the complexity and possible scenarios when a device goes down
	*
	*	So if we not get a state sync after a while, check all heads again and turn them off if necessary
	*
	*/
	void SignalDecoder::State_WaitTurnOff::OnThink(const dcclite::Clock::TimePoint_t time)
	{
		//restart again
		m_lstConnections.clear();
		m_uWaitListSize = 0;

		this->Init();

		//Somehow, all heads are off now... so go to next state
		if (m_uWaitListSize == 0)
		{
			this->GotoNextState();

			//we probably are dead now, so just jump off...
			return;
		}

		m_clTimeoutThinker.Schedule(time + WAIT_STATE_TIMEOUT);
	}

	void SignalDecoder::State_WaitTurnOff::OnDecoderStateSync(RemoteDecoder &decoder)
	{
		if (m_uWaitListSize == 0)
		{
			dcclite::Log::Error("[SignalDecoder::{}] [State_WaitTurnOff::OnDecoderStateSync] m_uWaitListSize == 0!! how?", decoder.GetName());

			return;
		}

		--m_uWaitListSize;

		if (m_uWaitListSize == 0)
		{
			this->GotoNextState();
		}
	}

	void SignalDecoder::State_WaitTurnOff::GotoNextState()
	{
		m_rclOwner.ForEachHead(m_rclOwner.m_vecAspects[m_rclOwner.m_uCurrentAspectIndex].m_vecOnHeads, m_rclOwner.m_eCurrentAspect, [this](OutputDecoder &dec)
			{
				dec.Activate(m_rclOwner.GetName().GetData().data());

				return true;
			}
		);

		if (m_rclOwner.m_vecAspects[m_rclOwner.m_uCurrentAspectIndex].m_Flash)
		{
			m_rclOwner.m_vState.emplace<State_Flash>(m_rclOwner, dcclite::Clock::DefaultClock_t::now());
		}
		else
		{
			m_rclOwner.m_vState.emplace<std::monostate>();
		}
	}
	
	
	SignalDecoder::State_Flash::State_Flash(SignalDecoder &self, const dcclite::Clock::TimePoint_t time):
		State{self},
		m_clThinker{ "SignalDecoder::State_Flash", THINKER_MF_LAMBDA(OnThink)}
	{
		this->OnThink(time);
	}
	
	void SignalDecoder::State_Flash::OnThink(const dcclite::Clock::TimePoint_t time)
	{		
		m_fOn = !m_fOn;

		const auto state = m_fOn ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE;

		m_rclOwner.ForEachHead(m_rclOwner.m_vecAspects[m_rclOwner.m_uCurrentAspectIndex].m_vecOnHeads, m_rclOwner.m_eCurrentAspect, [state, this](OutputDecoder &dec)
			{
				dec.SetState(state, m_rclOwner.GetName().GetData().data());

				return true;
			}
		);

		m_clThinker.Schedule(time + FLASH_INTERVAL);		
	}		
}