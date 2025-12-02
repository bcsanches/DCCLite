// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <dcclite/IFolderObject.h>

namespace dcclite::broker::tycoon
{
	class Cargo: public IObject
	{
		public:
			Cargo(RName name) :
				IObject{ name }
			{
				//empty
			}
		private:
	};

	class CargoHolder
	{		
		public:
			CargoHolder(const Cargo &cargo, float dailyRate, uint8_t maxQuantity) :
				m_rclCargo{ cargo },
				m_fDailyRate{ dailyRate },
				m_uMaxQuantity{ maxQuantity }
				
			{
				if(dailyRate <= 0.0f)
				{
					throw std::invalid_argument("[CargoHolder::CargoHolder] dailyRate must be greater than zero");
				}

				if(maxQuantity == 0)
				{
					throw std::invalid_argument("[CargoHolder::CargoHolder] maxQuantity must be greater than zero");
				}
			}

			void Consume()
			{
				if(m_uCurrentQuantity == 0)
				{
					throw std::runtime_error("[CargoHolder::Consume] No cargo available to consume");
				}

				--m_uCurrentQuantity;

				//FIXME: start production if not producing...
			}
			
		private:
			const Cargo &m_rclCargo;
			float		m_fDailyRate;
			uint8_t		m_uMaxQuantity;

			uint8_t		m_uCurrentQuantity = 0;
	};

	class Spot: public IObject
	{
		public:
			Spot(RName name) :
				IObject{ name }
			{
				//empty
			}

		private:

	};

	class Industry : public IFolderObject
	{
		public:
			Industry(RName name) :
				IFolderObject{ name }
			{
				//empty
			}
	};
}
