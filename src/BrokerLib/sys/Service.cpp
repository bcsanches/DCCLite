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
	void Service::NotifyItemCreated(dcclite::IItem &item) const
	{
		ObjectManagerEvent ev(
			ObjectManagerEvent::ITEM_CREATED,
			*this,
			&item
		);

		this->DispatchEvent(ev);
	}

	void Service::NotifyItemDestroyed(dcclite::IItem &item) const
	{
		ObjectManagerEvent ev(
			ObjectManagerEvent::ITEM_DESTROYED,
			*this,
			&item
		);

		this->DispatchEvent(ev);
	}

	void Service::NotifyItemChanged(dcclite::IItem &item, ObjectManagerEvent::SerializeDeltaProc_t proc) const
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
		m_sigEvent(event);		
	}
}


