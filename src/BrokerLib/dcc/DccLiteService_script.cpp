// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DccLiteService.h"

#include <magic_enum/magic_enum.hpp>

#include <dcclite/FmtUtils.h>
#include <dcclite/Log.h>
#include <dcclite/Nmra.h>

#include "RemoteDecoder.h"
#include "SignalDecoder.h"
#include "TurnoutDecoder.h"

/******************************************************************************
*
* DecoderProxy
*
* Represents a decoder inside the script VM
*
* It needs to track when decoders are created and destroyed during editing
*
* We simple assume that on regular use decoders will be stable, so we ignore for
* example, if a decoder is never recreated... this will leave a dummy proxy on the
* VM
*
******************************************************************************/
class DecoderProxy
{
	public:
		DecoderProxy(dcclite::broker::Decoder &decoder, dcclite::broker::Service &dccLiteService):
			m_rclDecoder{ decoder }
		{
			dcclite::Log::Trace("[ScriptService] [DecoderProxy] [{}]: Created.", decoder.GetName());			
		}

		~DecoderProxy()
		{
			dcclite::Log::Trace("[ScriptService] [DecoderProxy] [{}]: Destructor called", this->GetName());
		}

		inline const uint16_t GetAddress() const
		{
			return m_rclDecoder.GetAddress().GetAddress();
		}

		inline std::string_view GetName() const
		{
			return m_rclDecoder.GetName().GetData();
		}

		inline bool IsActive() const
		{
			auto turnout = DynamicDecoderCast<dcclite::broker::StateDecoder>();

			return turnout->GetState() == dcclite::DecoderStates::ACTIVE;
		}

		void SetState(bool active)
		{
			auto decoder = DynamicDecoderCast<dcclite::broker::OutputDecoder>();

			decoder->SetState(active ? dcclite::DecoderStates::ACTIVE : dcclite::DecoderStates::INACTIVE, "DecoderProxy");
		}

		inline bool IsInactive() const
		{
			return !this->IsActive();
		}

		void OnStateChange(sol::function callBack)
		{
			auto it = std::find(m_vStateChangeCallbacks.begin(), m_vStateChangeCallbacks.end(), callBack);
			if (it != m_vStateChangeCallbacks.end())
			{
				dcclite::Log::Warn("[ScriptService] [DecoderProxy::OnStateChange] [{}] Callback already registered", this->GetName());

				return;
			}

			m_vStateChangeCallbacks.push_back(callBack);

			if (!m_slotRemoteDecoderStateSyncConnection.connected())
				this->RegisterStateSyncCallback();
		}

		void SetAspect(dcclite::SignalAspects aspect, std::string requester, std::string reason)
		{
			auto decoder = DynamicDecoderCast<dcclite::broker::SignalDecoder>();

			decoder->SetAspect(aspect, std::move(requester), std::move(reason));
		}

		dcclite::SignalAspects GetAspect()
		{
			auto decoder = DynamicDecoderCast<dcclite::broker::SignalDecoder>();

			return decoder->GetAspect();
		}

	private:
		template <typename T>
		inline T *DynamicDecoderCast()
		{
			auto decoder = dynamic_cast<T *>(&m_rclDecoder);
			if (!decoder)
			{
				throw std::runtime_error(fmt::format("[ScriptService] [DecoderProxy::SetState] [{}]: Decoder is not an output decoder {}!!!", this->GetName(), typeid(T).name()));
			}

			return decoder;
		}

		template <typename T>
		inline const T *DynamicDecoderCast() const
		{
			auto decoder = dynamic_cast<const T *>(&m_rclDecoder);
			if (!decoder)
			{
				throw std::runtime_error(fmt::format("[ScriptService] [DecoderProxy::SetState] [{}]: Decoder is not an output decoder {}!!!", this->GetName(), typeid(T).name()));
			}

			return decoder;
		}

		void OnRemoteDecoderStateSync(dcclite::broker::RemoteDecoder &decoder)
		{
			for (auto f : m_vStateChangeCallbacks)
			{
				auto r = f.call(std::ref(*this));

				if (!r.valid())
				{
					sol::error err = r;

					dcclite::Log::Error("[ScriptService] [DecoderProxy::OnRemoteDecoderStateSync] Call failed with result: {} - {}", magic_enum::enum_name(r.status()), err.what());
				}
			}
		}

		void RegisterStateSyncCallback()
		{
			auto remoteDecoder = this->DynamicDecoderCast<dcclite::broker::RemoteDecoder>();

			m_slotRemoteDecoderStateSyncConnection = remoteDecoder->m_sigRemoteStateSync.connect(&DecoderProxy::OnRemoteDecoderStateSync, this);
		}

		dcclite::broker::Decoder	&m_rclDecoder;

		sigslot::scoped_connection	m_slotRemoteDecoderStateSyncConnection;		

		std::vector<sol::protected_function> m_vStateChangeCallbacks;
};

/******************************************************************************
*
* DccLiteProxy
*
* We put all the scripting funtionality on a helper class for better code isolation
* and to reduce dependencies.
*
******************************************************************************/
class DccLiteProxy
{
public:
	DccLiteProxy(dcclite::broker::DccLiteService &service):
		m_rclService{ service }
	{
		dcclite::Log::Trace("[ScriptService] [DccLiteProxy] [{}]: Created.", service.GetName());
	}

	DccLiteProxy(DccLiteProxy &&m) = default;

	DccLiteProxy(const DccLiteProxy &) = delete;
	DccLiteProxy operator=(const DccLiteProxy &) = delete;

	~DccLiteProxy()
	{
		dcclite::Log::Trace("[ScriptService] [DccLiteProxy] [{}] Destroyed.", this->m_rclService.GetName());
	}

	DecoderProxy *OnIndexByName(std::string_view key, sol::this_state L);
	DecoderProxy *OnIndexByAddress(uint16_t key, sol::this_state L);

private:
	dcclite::broker::DccLiteService &m_rclService;

	std::map<dcclite::broker::DccAddress, std::unique_ptr<DecoderProxy>> m_mapKnowDecoders;
};

DecoderProxy *DccLiteProxy::OnIndexByName(std::string_view key, sol::this_state L)
{
	const auto decoder = this->m_rclService.TryFindDecoder(dcclite::RName{ key });

	if (!decoder)
	{
		dcclite::Log::Error("[ScriptService] [DccLiteProxy] [{}]: Trying to access non existent decoder: {}", m_rclService.GetName(), key);

		return nullptr;
	}

	const auto decAddress = decoder->GetAddress();
	auto it = m_mapKnowDecoders.lower_bound(decAddress);

	if ((it != m_mapKnowDecoders.end()) && !(m_mapKnowDecoders.key_comp()(decAddress, it->first)))
	{
		return it->second.get();
	}
	else
	{
		it = m_mapKnowDecoders.emplace_hint(it, decAddress, std::make_unique<DecoderProxy>(*decoder, m_rclService));

		return it->second.get();
	}
}

DecoderProxy *DccLiteProxy::OnIndexByAddress(uint16_t key, sol::this_state L)
{
	const auto decAddress = dcclite::broker::DccAddress{ key };

	auto it = m_mapKnowDecoders.lower_bound(decAddress);

	if ((it != m_mapKnowDecoders.end()) && !(m_mapKnowDecoders.key_comp()(decAddress, it->first)))
	{
		return it->second.get();
	}
	else
	{
		auto decoder = this->m_rclService.TryFindDecoder(decAddress);
		if (!decoder)
		{
			dcclite::Log::Error("[ScriptService] [DccLiteProxy] [{}]: Trying to access non existent decoder: {}", m_rclService.GetName(), key);

			return nullptr;
		}

		it = m_mapKnowDecoders.emplace_hint(it, decAddress, std::make_unique<DecoderProxy>(*decoder, m_rclService));

		return it->second.get();
	}
}

namespace dcclite::broker
{
	void DccLiteService::IScriptSupport_RegisterProxy(sol::table &table)
	{
		table[this->GetName().GetData()] = DccLiteProxy{ *this };
	}

	void DccLiteService::IScriptSupport_OnVMInit(sol::state &state)
	{
		state.new_usertype<DccLiteProxy>(
			"dcclite_service",
			sol::no_constructor,
			sol::meta_function::index,
			sol::overload(&DccLiteProxy::OnIndexByAddress, &DccLiteProxy::OnIndexByName)
		);

		state.new_usertype<DecoderProxy>(
			"decoder", sol::no_constructor,
			"name", sol::property(&DecoderProxy::GetName),
			"address", sol::property(&DecoderProxy::GetAddress),
			"thrown", sol::property(&DecoderProxy::IsActive),
			"closed", sol::property(&DecoderProxy::IsInactive),
			"active", sol::property(&DecoderProxy::IsActive),
			"inactive", sol::property(&DecoderProxy::IsInactive),
			"state", sol::property(&DecoderProxy::IsActive),
			"set_state", &DecoderProxy::SetState,
			"on_state_change", &DecoderProxy::OnStateChange,
			"set_aspect", &DecoderProxy::SetAspect,
			"aspect", sol::property(&DecoderProxy::GetAspect)
		);

		state.new_enum<dcclite::SignalAspects>(
			"SignalAspects",
			{
				{magic_enum::enum_name(SignalAspects::Stop),					SignalAspects::Stop},
				{magic_enum::enum_name(SignalAspects::TakeSiding),				SignalAspects::TakeSiding},
				{magic_enum::enum_name(SignalAspects::StopOrders),				SignalAspects::StopOrders},
				{magic_enum::enum_name(SignalAspects::StopProceed),				SignalAspects::StopProceed},
				{magic_enum::enum_name(SignalAspects::Restricted),				SignalAspects::Restricted},
				{magic_enum::enum_name(SignalAspects::SlowAproach),				SignalAspects::SlowAproach},
				{magic_enum::enum_name(SignalAspects::Slow),					SignalAspects::Slow},
				{magic_enum::enum_name(SignalAspects::MediumAproach),			SignalAspects::MediumAproach},
				{magic_enum::enum_name(SignalAspects::MediumSlow),				SignalAspects::MediumSlow},
				{magic_enum::enum_name(SignalAspects::Medium),					SignalAspects::Medium},
				{magic_enum::enum_name(SignalAspects::MediumLimited),			SignalAspects::MediumLimited},
				{magic_enum::enum_name(SignalAspects::MediumClear),				SignalAspects::MediumClear},
				{magic_enum::enum_name(SignalAspects::LimitedAproach),			SignalAspects::LimitedAproach},
				{magic_enum::enum_name(SignalAspects::LimitedSlow),				SignalAspects::LimitedSlow},
				{magic_enum::enum_name(SignalAspects::LimitedMedium),			SignalAspects::LimitedMedium},
				{magic_enum::enum_name(SignalAspects::Limited),					SignalAspects::Limited},
				{magic_enum::enum_name(SignalAspects::LimitedClear),			SignalAspects::LimitedClear},
				{magic_enum::enum_name(SignalAspects::Aproach),					SignalAspects::Aproach},
				{magic_enum::enum_name(SignalAspects::AdvanceAproach),			SignalAspects::AdvanceAproach},
				{magic_enum::enum_name(SignalAspects::AproachSlow),				SignalAspects::AproachSlow},
				{magic_enum::enum_name(SignalAspects::AdvanceAproachSlow),		SignalAspects::AdvanceAproachSlow},
				{magic_enum::enum_name(SignalAspects::AproachMedium),			SignalAspects::AproachMedium},
				{magic_enum::enum_name(SignalAspects::AdvanceAproachMedium),	SignalAspects::AdvanceAproachMedium},
				{magic_enum::enum_name(SignalAspects::AproachLimited),			SignalAspects::AproachLimited},
				{magic_enum::enum_name(SignalAspects::AdvanceAproachLimited),	SignalAspects::AdvanceAproachLimited},
				{magic_enum::enum_name(SignalAspects::Clear),					SignalAspects::Clear},
				{magic_enum::enum_name(SignalAspects::CabSpeed),				SignalAspects::CabSpeed},
				{magic_enum::enum_name(SignalAspects::Dark),					SignalAspects::Dark}
			}
		);
	}

	void DccLiteService::IScriptSupport_OnVMFinalize(sol::state &state)
	{

	}
}

