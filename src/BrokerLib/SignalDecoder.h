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

#include "Decoder.h"

#include<functional>
#include <set>
#include <variant>
#include <vector>

#include <Clock.h>

#include "SharedLibDefs.h"
#include "NmraUtil.h"

class SignalTester;


namespace dcclite::broker
{
	class OutputDecoder;


	class SignalDecoder : public Decoder
	{
		public:
			SignalDecoder(			
				const DccAddress &address,
				const std::string &name,
				IDccLite_DecoderServices &owner,
				IDevice_DecoderServices &dev,
				const rapidjson::Value &params
			);

			//
			//IObject
			//
			//

			const char* GetTypeName() const noexcept override
			{
				return "SignalDecoder";
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			//
			//
			//
			//
			//

			void SetAspect(const dcclite::SignalAspects aspect, const char *requester);

			void Update(const dcclite::Clock &clock);

		private:
			void ForEachHead(const std::vector<std::string> &heads, const dcclite::SignalAspects aspect, std::function<bool(OutputDecoder &)> proc) const;

		private:
			struct Aspect
			{			
				dcclite::SignalAspects m_eAspect;

				std::vector<std::string> m_vecOnHeads;
				std::vector<std::string> m_vecOffHeads;

				bool m_Flash = false;
			};

			friend class ::SignalTester;

			struct State
			{
				virtual void Update(SignalDecoder &self, const dcclite::Clock::TimePoint_t time) = 0;
			};

			struct State_TurnOff : State
			{
				void Update(SignalDecoder &self, const dcclite::Clock::TimePoint_t time) override;
			};

			struct State_WaitTurnOff: State
			{
				void Update(SignalDecoder &self, const dcclite::Clock::TimePoint_t time) override;
			};

			struct State_Flash : State
			{
				State_Flash(const dcclite::Clock::TimePoint_t time);

				void Update(SignalDecoder &self, const dcclite::Clock::TimePoint_t time) override;

				bool m_fOn = true;
				dcclite::Clock::TimePoint_t m_tNextThink;
			};

			struct NullState {};

		private:
			dcclite::SignalAspects	m_eCurrentAspect;
			size_t					m_uCurrentAspectIndex;

			std::variant<NullState, State_Flash, State_WaitTurnOff, State_TurnOff> m_vState;
			State *m_pclCurrentState = nullptr;

			std::map<std::string, std::string> m_mapHeads;
			std::vector<Aspect> m_vecAspects;
	};

}