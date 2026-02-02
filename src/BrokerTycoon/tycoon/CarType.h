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

#include <dcclite/Object.h>

namespace dcclite::broker::tycoon
{
	class Cargo;

	class CarType : public dcclite::INamedItem
	{
		public:
			CarType(RName name, RName model, std::string description) :
				INamedItem{ name },
				m_nModel{ model },
				m_strDescription{ std::move(description) }
			{
				//empty
			}

			CarType(CarType &&rhs) noexcept = default;
			CarType &operator=(CarType &&rhs) noexcept = default;

			void AddCargo(const Cargo &cargo);

			[[nodiscard]] inline bool HasAnyCargo() const noexcept
			{
				return !m_vecCargos.empty();
			}
		
			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

		private:
			RName		m_nModel;
			std::string m_strDescription;

			std::vector<const Cargo *> m_vecCargos;
	};
}

