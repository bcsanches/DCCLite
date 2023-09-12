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

#include <vector>

#include "Object.h"

#include <rapidjson/document.h>

namespace dcclite::broker
{

	class Decoder;
	class Location;

	enum class LocationMismatchReason
	{
		WRONG_LOCATION_HINT,
		OUTSIDE_RANGES
	};

	class LocationManager: public dcclite::FolderObject
	{	
		public:
			LocationManager(RName name, const rapidjson::Value& params);
			~LocationManager() override
			{
				//empty
			}

			void RegisterDecoder(const Decoder &decoder);
			void UnregisterDecoder(const Decoder &decoder);

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			const char *GetTypeName() const noexcept override
			{
				return "LocationManager";
			}

		private:
			std::vector<Location *> m_vecIndex;

			std::vector<std::tuple<const Decoder *, LocationMismatchReason, const Location *>> m_vecMismatches;
	};
}