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

#include <functional>
#include <list>
#include <map>
#include <variant>
#include <vector>

#include <dcclite/Clock.h>
#include <dcclite/Nmra.h>

#include "sigslot/signal.hpp"

#include "sys/Thinker.h"

class SignalTester;


namespace dcclite::broker::exec::dcc
{
	class OutputDecoder;
	class RemoteDecoder;


	/**
	
	A signal decoder represents a virtual signal, actually this is a helper class to help managing signals.

	A signal is formed by a list of heads, that contains a list of user named output decoders, each one represents a "lamp" 
	for a signal. It can be any output decoder


	After the heads definitions, come the aspects definitions, each aspect contains:
		- name: name of the aspect, ie "Stop" (any value from SignalAspects enum)
		- two possible list of heads states:
			on: list of heads to turn on when this aspect is active
			off: list of hedas to turn off when this aspect is active

		Only one list is required. If just one list is specified, the other list is automatically filled with all other heads	
	*/
	class SignalDecoder : public Decoder
	{
		public:
			SignalDecoder(			
				const Address &address,
				RName name,
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

			void SetAspect(const dcclite::SignalAspects aspect, std::string requester, std::string reason);
			inline dcclite::SignalAspects GetAspect() const
			{
				return m_eCurrentAspect;
			}

		private:
			void ForEachHead(const std::vector<RName> &heads, const dcclite::SignalAspects aspect, std::function<bool(OutputDecoder &)> proc) const;

			void ApplyAspect(const dcclite::SignalAspects aspect, const unsigned aspectIndex);

		private:
			struct Aspect
			{			
				dcclite::SignalAspects m_kAspect;

				std::vector<RName> m_vecOnHeads;
				std::vector<RName> m_vecOffHeads;

				bool m_Flash = false;
			};

			friend class ::SignalTester;		

			struct State
			{
				explicit State(SignalDecoder &owner):
					m_rclOwner{ owner }
				{
					//empty
				}

				SignalDecoder &m_rclOwner;
			};

			struct State_WaitTurnOff: State
			{				
				explicit State_WaitTurnOff(SignalDecoder &self);

				void GotoNextState();

				[[nodiscard]]
				inline unsigned GetWaitListSize() const noexcept
				{
					return m_uWaitListSize;
				}

				private:
					void OnDecoderStateSync(RemoteDecoder &decoder);					

					void OnThink(const dcclite::Clock::TimePoint_t time);

					void Init();

					std::list<sigslot::scoped_connection> m_lstConnections;
					unsigned m_uWaitListSize = 0;

					sys::Thinker m_clTimeoutThinker;
			};

			struct State_Flash: State
			{
				State_Flash(SignalDecoder &self, const dcclite::Clock::TimePoint_t time);

				void OnThink(const dcclite::Clock::TimePoint_t time);

				bool m_fOn = true;				

				sys::Thinker m_clThinker;
			};

		private:
			dcclite::SignalAspects	m_eCurrentAspect;
			unsigned				m_uCurrentAspectIndex;

			std::string				m_strAspectReason;
			std::string				m_strAspectRequester;

			std::variant<std::monostate, State_Flash, State_WaitTurnOff> m_vState;			

			//Heads definitions, std::map key is the head "user name" and std::map value is the decoder name
			std::map<dcclite::RName, dcclite::RName> m_mapHeads;

			//User defined aspects
			std::vector<Aspect> m_vecAspects;
	};

}