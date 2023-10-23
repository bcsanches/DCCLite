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
#include <vector>

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
	typedef uint32_t NameIndexType_t;

	class RName;

	namespace detail
	{
		class RNameState;

		struct NameIndex
		{			
			NameIndexType_t m_uIndex = 0;

			inline bool operator ==(const NameIndex &index) const
			{
				return (m_uIndex == index.m_uIndex);
			}

			inline bool operator !=(const NameIndex &index) const
			{
				return (m_uIndex != index.m_uIndex);
			}

			inline bool operator <(const NameIndex &index) const
			{
				return (m_uIndex < index.m_uIndex);
			}

			explicit operator bool() const
			{
				return m_uIndex;
			}
		};

		struct RNameClusterInfo
		{			
			uint32_t m_uNumChars;

			uint32_t m_uRoomLeft;			
		};

		uint16_t RName_GetNumClusters() noexcept;

		RNameClusterInfo RName_GetClusterInfo(uint16_t cluster);

		std::vector<RName> RName_GetAll();

		void RName_ForceNewCluster();
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
				return m_stIndex.m_uIndex;
			}

			const std::string_view GetData() const;

			//
			//
			// Helpers for unit testing, should be useless on regular systems
			//			
			// Those are internal data, interface is not guarantee, so you SHOULD not use it or depend on it...
			//

			inline NameIndexType_t GetIndex() const
			{
				return m_stIndex.m_uIndex;
			}

			uint32_t FindCluster() const;

			//returns cluster id (first) and position inside cluster
			std::pair<uint32_t, uint32_t> FindClusterInfo() const;

		private:			
#ifdef DCCLITE_DEBUG
			RName(detail::NameIndex index, std::string_view data) :
				m_svName{data},
#else
			RName(detail::NameIndex index) :
#endif
				m_stIndex{ index }
			{
				//empty
			}

			friend class detail::RNameState;

		private:
			detail::NameIndex m_stIndex;

#ifdef DCCLITE_DEBUG
			std::string_view m_svName;
#endif
	};
}
