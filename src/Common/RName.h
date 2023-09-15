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

#include <string_view>
#include <optional>

/**
* 
* Registered Name System (RName)
* 
* A simple way for caching strings used as names. 
* 
* It guarantees that the strings will be kept forever while the program is running, there is no way to remove 
* a string from the cache. Once registered, it goes forever
* 
* Strings are assumed to be imutable, so do not change then or behavior will be undefined
* 
*/

namespace dcclite
{
	namespace detail
	{
		class RNameState;

		struct NameIndex
		{
			uint16_t m_uCluster = 0;
			uint16_t m_uIndex = 0;

			inline bool operator ==(const NameIndex &index) const
			{
				return (m_uCluster == index.m_uCluster) && (m_uIndex == index.m_uIndex);
			}

			inline bool operator !=(const NameIndex &index) const
			{
				return (m_uCluster != index.m_uCluster) || (m_uIndex != index.m_uIndex);
			}

			inline bool operator <(const NameIndex &index) const
			{
				return (m_uCluster == index.m_uCluster) ? (m_uIndex < index.m_uIndex) : (m_uCluster < index.m_uCluster);
			}

			explicit operator bool() const
			{
				return m_uCluster || m_uIndex;
			}
		};

		struct RNameClusterInfo
		{
			uint32_t m_uNumNames;
			uint32_t m_uNumChars;

			uint32_t m_uRoomLeft;

			uint32_t m_uRoomForNamesLeft;
		};

		uint16_t RName_GetNumClusters() noexcept;

		RNameClusterInfo RName_GetClusterInfo(uint16_t cluster);
	}

	class RName
	{
		public:
			static RName TryGetName(std::string_view name);

			//
			// We only allow creation thought static methods to make it clear when it is registering and just grabbing
			//
			static RName Get(std::string_view name);
			inline static RName Create(std::string_view name)
			{
				return RName{ name };
			}

			explicit RName(std::string_view name);

			inline RName()				
			{
				//empty	
			}			

			RName(const RName &) = default;
			RName(RName &&) = default;

			RName &operator=(const RName &rhs) = default;
			RName &operator=(RName &&rhs) = default;

			inline bool operator==(const RName &rhs) const
			{
				return m_stIndex == rhs.m_stIndex;
			}

			inline bool operator!=(const RName &rhs) const
			{
				return m_stIndex != rhs.m_stIndex;
			}

			//This is not lexicographic, just for comparing indices....
			inline bool operator<(const RName &rhs) const
			{
				return m_stIndex < rhs.m_stIndex;
			}

			explicit operator bool() const
			{
				return m_stIndex.m_uCluster || m_stIndex.m_uIndex;
			}

			const std::string_view GetData() const;

			//
			//
			// Helpers for unit testing, should be useless on regular systems
			//

			inline uint16_t GetCluster() const noexcept
			{
				return m_stIndex.m_uCluster;
			}

			inline uint16_t GetIndex() const
			{
				return m_stIndex.m_uIndex;
			}

		private:			
			RName(detail::NameIndex index) :
				m_stIndex{ index }
			{
				//empty
			}

			friend class detail::RNameState;

		private:
			detail::NameIndex m_stIndex;
	};
}
