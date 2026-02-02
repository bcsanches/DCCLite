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

#include <dcclite/FileSystem.h>

#include <sys/Service.h>

#include "FastClock.h"

namespace dcclite::broker::tycoon
{
	class Cargo;
	class CarType;
	class Industry;
	class IndustryToken;

	class TycoonService : public sys::Service
	{
		public:
			static void RegisterFactory();

			static const char *TYPE_NAME;
	
			TycoonService(RName name, sys::Broker &broker, const rapidjson::Value &params);

			const Cargo *TryFindCargoByName(RName name) const noexcept;
			const Cargo &FindCargoByName(RName name) const;

			const FastClock &GetFastClock() const noexcept
			{
				return m_clFastClock;
			}

			FastClock &GetFastClock() noexcept
			{
				return m_clFastClock;
			}

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			const char *GetTypeName() const noexcept override
			{
				return TYPE_NAME;
			}

			void OnObjectStateChanged(const IndustryToken &token, const Industry &industry);

		private:	
			void Load(const rapidjson::Value &params);
			void LoadCargos(const rapidjson::Value &params);
			void LoadCarTypes(const rapidjson::Value &params);
			void LoadLocations(const rapidjson::Value &params);			

			void AddCargoToCarType(CarType &carType, std::string_view cargoName);			

		private:
			FastClock				m_clFastClock;

			std::vector<Cargo>		m_vecCargos;
			std::vector<CarType>	m_vecCarTypes;

			dcclite::fs::path		m_pathDataFileName;

			FolderObject			*m_pLocations;
	};
}
