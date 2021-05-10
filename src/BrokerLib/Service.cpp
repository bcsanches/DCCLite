// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Service.h"

namespace dcclite::broker
{
	void Service::AddListener(IObjectManagerListener &listener)
	{
		m_vecListeners.push_back(&listener);
	}

	void Service::RemoveListener(IObjectManagerListener &listener)
	{
		m_vecListeners.erase(std::remove(m_vecListeners.begin(), m_vecListeners.end(), &listener), m_vecListeners.end());
	}

	void Service::NotifyItemCreated(const dcclite::IItem &item) const
	{
		ObjectManagerEvent ev(
			ObjectManagerEvent::ITEM_CREATED,
			*this,
			&item
		);

		this->DispatchEvent(ev);
	}

	void Service::NotifyItemDestroyed(const dcclite::IItem &item) const
	{
		ObjectManagerEvent ev(
			ObjectManagerEvent::ITEM_DESTROYED,
			*this,
			&item
		);

		this->DispatchEvent(ev);
	}

	void Service::NotifyItemChanged(const dcclite::IItem &item, ObjectManagerEvent::SerializeDeltaProc_t proc) const
	{
		ObjectManagerEvent ev(
			ObjectManagerEvent::ITEM_CHANGED,
			*this,
			&item,
			proc
		);

		this->DispatchEvent(ev);
	}

	void Service::DispatchEvent(const ObjectManagerEvent &event) const
	{
		for (auto listener : m_vecListeners)
		{
			listener->OnObjectManagerEvent(event);
		}
	}
}


