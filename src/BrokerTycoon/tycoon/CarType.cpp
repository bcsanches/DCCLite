// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "CarType.h"

#include <fmt/format.h>

#include "Cargo.h"

namespace dcclite::broker::tycoon
{
	void CarType::AddCargo(const Cargo &cargo)
	{
		if(std::ranges::find(m_vecCargos, &cargo) != m_vecCargos.end())
		{
			throw std::invalid_argument(
				fmt::format(
					"[CarType::{}] Cargo '{}' is already added to car type", 
					this->GetName().GetData(),
					cargo.GetName().GetData()
				)
			);
		}

		m_vecCargos.emplace_back(&cargo);
	}
}

