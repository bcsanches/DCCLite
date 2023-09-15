// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "RName.h"

#include <cassert>
#include <array>
#include <map>
#include <mutex>
#include <vector>

#include <city.h>

#include <fmt/format.h>

#define MAX_NAMES		64
#define MAX_NAME_LEN	512

constexpr auto DATA_BUFFER_SIZE = MAX_NAMES * MAX_NAME_LEN;

namespace dcclite::detail
{

	struct NameData
	{
		uint16_t m_uPosition;
		uint16_t m_uSize;		
	};

	struct Cluster
	{
		std::array<char, DATA_BUFFER_SIZE>			m_arNames;
		uint32_t									m_uPosition;

		std::array<NameData, MAX_NAMES>				m_arInfo;
		uint32_t									m_uInfoIndex;
	};	

	static_assert(DATA_BUFFER_SIZE <= std::numeric_limits<uint16_t>::max());

	class RNameState
	{
		public:
			RNameState();

			NameIndex RegisterName(std::string_view name);

			inline std::string_view GetName(detail::NameIndex index) const
			{
				assert(index.m_uCluster < m_vecClusters.size());
				assert(index.m_uIndex < m_vecClusters[index.m_uCluster]->m_uInfoIndex);

				auto cluster = m_vecClusters[index.m_uCluster].get();

				auto &info = cluster->m_arInfo[index.m_uIndex];

				return std::string_view{ &cluster->m_arNames[info.m_uPosition], info.m_uSize };
			}

			inline dcclite::RName TryGetName(std::string_view name);

			inline uint32_t GetNumClusters() const
			{
				return static_cast<uint32_t>(m_vecClusters.size());
			}

			inline dcclite::detail::RNameClusterInfo GetClusterInfo(uint16_t index)
			{
				auto cluster = m_vecClusters.at(index).get();

				dcclite::detail::RNameClusterInfo info;

				info.m_uNumChars = cluster->m_uPosition;
				info.m_uNumNames = cluster->m_uInfoIndex;
				info.m_uRoomLeft = static_cast<uint32_t>(cluster->m_arNames.size()) - cluster->m_uPosition;
				info.m_uRoomForNamesLeft = static_cast<uint32_t>(cluster->m_arInfo.size()) - cluster->m_uInfoIndex;

				return info;
			}
			

		private:
			std::vector<std::unique_ptr<Cluster>>		m_vecClusters;

			std::map<uint64_t, NameIndex>				m_mapIndex;

			std::mutex									m_clLock;
	};

	RNameState::RNameState()
	{		
		//consume position 0 with null....
		this->RegisterName("null_name");
	}

	inline dcclite::RName RNameState::TryGetName(std::string_view name)
	{
		auto hash = CityHash64(name.data(), name.size());

		std::unique_lock lock{ m_clLock };		

		auto it = m_mapIndex.find(hash);
		if (it != m_mapIndex.end())
		{
			//name already registered?
			return RName{ it->second };
		}

		return RName{};
	}

	NameIndex RNameState::RegisterName(std::string_view name)
	{		
		auto hash = CityHash64(name.data(), name.size());

		std::unique_lock lock{ m_clLock };

		auto it = m_mapIndex.find(hash);
		if (it != m_mapIndex.end())
		{
			//name already registered?
			return it->second;
		}

		Cluster *cluster = nullptr;
		size_t clusterIndex = 0;

		for (;;)
		{			
			for (auto len = m_vecClusters.size(); clusterIndex < len; ++clusterIndex)
			{
				auto item = m_vecClusters[clusterIndex].get();
				if (item->m_uInfoIndex == item->m_arInfo.size())
				{
					continue;
				}

				cluster = item;
				break;
			}

			//all clusters full?
			if (!cluster)
			{
				m_vecClusters.emplace_back(new Cluster());
				cluster = m_vecClusters.back().get();

				clusterIndex = m_vecClusters.size() - 1;
			}

			//plus \0
			const auto len = name.length();

			//should not be necessary, as it will not fit on array... but just in case we change this array size later and forget this...
			if (len >= std::numeric_limits<uint16_t>().max())
			{
				throw std::runtime_error(fmt::format("[RNameState::TryRegisterName] Name {} is too big", name));
			}

			//does the data fits on the buffer?
			if ((len + 1) + cluster->m_uPosition >= cluster->m_arNames.size())
			{
				//cluster is full and no more clusters? - UnitTest LimitTest should test this condition
				if (clusterIndex < m_vecClusters.size() - 1)
				{
					++clusterIndex;
					cluster = nullptr;

					//try next cluster
					continue;
				}

				m_vecClusters.emplace_back(new Cluster());
				cluster = m_vecClusters.back().get();

				clusterIndex = m_vecClusters.size() - 1;
			}

			cluster->m_arInfo[cluster->m_uInfoIndex].m_uPosition = cluster->m_uPosition;
			cluster->m_arInfo[cluster->m_uInfoIndex].m_uSize = static_cast<uint16_t>(len);

			const auto startPosition = cluster->m_uPosition;

			strncpy(&cluster->m_arNames[cluster->m_uPosition], name.data(), len);
			cluster->m_uPosition += static_cast<uint32_t>(name.size());
			cluster->m_arNames[cluster->m_uPosition++] = '\0';

			NameIndex index;
			index.m_uCluster = static_cast<uint16_t>(clusterIndex);
			index.m_uIndex = cluster->m_uInfoIndex++;

			m_mapIndex[hash] = index;

			return index;
		}
	}

	static RNameState &GetState()
	{
		static RNameState state;

		return state;
	}
}

namespace dcclite
{
	RName::RName(std::string_view name) :
		m_stIndex{ detail::GetState().RegisterName(name) }
	{
		//empty
	}

	std::string_view RName::GetData() const
	{
		return detail::GetState().GetName(m_stIndex);
	}

	RName RName::TryGetName(std::string_view name)
	{
		return detail::GetState().TryGetName(name);		
	}

	RName RName::Get(std::string_view name)
	{
		auto rname = RName::TryGetName(name);

		if (!rname)
			throw std::invalid_argument(fmt::format("[RName::GetName] Name \"{}\" is not registered", name));

		return rname;
	}	
}

namespace dcclite::detail
{
	uint16_t RName_GetNumClusters() noexcept
	{
		auto &state = GetState();

		return state.GetNumClusters();
	}

	RNameClusterInfo RName_GetClusterInfo(uint16_t cluster)
	{
		auto &state = GetState();

		return state.GetClusterInfo(cluster);
	}
}

