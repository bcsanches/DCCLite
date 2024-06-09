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

#include "DccAddress.h"

#include "IFolderObject.h"

#include <rapidjson/document.h>

namespace dcclite::broker
{
	class Decoder;	
	class LocationManager;

	enum class LocationMismatchReason
	{
		WRONG_LOCATION_HINT,
		OUTSIDE_RANGES
	};

	namespace detail
	{
		class Location : public IObject
		{
			public:
				Location(RName name, RName prefix, const DccAddress beginAddress, const DccAddress endAddress, LocationManager &owner);

				Location(const Location &) = default;
				Location(Location &&) = default;

#if 1
				Location &operator=(Location &&rhs) = default;
#else
				Location &operator=(Location &&rhs)
				{
					IObject::operator= (std::move(rhs));

					m_rnPrefix = std::move(rhs.m_rnPrefix);
					m_tBeginAddress = std::move(rhs.m_tBeginAddress);
					m_tEndAddress = std::move(rhs.m_tEndAddress);
					
					m_vecDecoders = std::move(rhs.m_vecDecoders);

					m_rclParent = rhs.m_rclParent;

					return *this;	
				}
#endif

				Location &operator=(const Location &) = default;

				void RegisterDecoder(const Decoder &dec);

				void UnregisterDecoder(const Decoder &dec);

				inline bool IsDecoderRegistered(const Decoder &dec) const
				{
					return m_vecDecoders[GetDecoderIndex(dec)] == &dec;
				}

				const char *GetTypeName() const noexcept override
				{
					return "Location";
				}

				void Serialize(JsonOutputStream_t &stream) const override;

				inline DccAddress GetBeginAddress() const
				{
					return m_tBeginAddress;
				}

				inline DccAddress GetEndAddress() const
				{
					return m_tEndAddress;
				}

				inline RName GetPrefix() const
				{
					return m_rnPrefix;
				}

				IFolderObject *GetParent() const noexcept override;				

			private:
				inline size_t GetDecoderIndex(const Decoder &dec) const;

			private:
				RName		m_rnPrefix;
				DccAddress	m_tBeginAddress;
				DccAddress	m_tEndAddress;

				std::vector<const Decoder *> m_vecDecoders;

				LocationManager *m_rclParent;
		};
	}

	class LocationManager: public dcclite::IFolderObject
	{	
		public:
			LocationManager(RName name, const rapidjson::Value& params);
			~LocationManager() override = default;

			void RegisterDecoder(const Decoder &decoder);
			void UnregisterDecoder(const Decoder &decoder);

			void Serialize(dcclite::JsonOutputStream_t &stream) const override;

			IObject *TryGetChild(RName name) override;

			void VisitChildren(Visitor_t visitor) override;

			const char *GetTypeName() const noexcept override
			{
				return "LocationManager";
			}

		private:
			std::vector<detail::Location> m_vecIndex;

			std::vector<std::tuple<const Decoder *, LocationMismatchReason, const detail::Location *>> m_vecMismatches;
	};
}