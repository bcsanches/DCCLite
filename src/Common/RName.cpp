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

#include "Log.h"

#define MAX_NAMES		64
#define MAX_NAME_LEN	512

#define CHAIN_SIZE		2

constexpr auto DATA_BUFFER_SIZE = MAX_NAMES * MAX_NAME_LEN;

namespace dcclite::detail
{	
	/// <summary>
	/// Just story the starting position of the name and its length on the cluster buffer
	/// </summary>
	struct NameData
	{
		uint16_t m_uPosition;
		uint16_t m_uSize;		
	};

	/**
	* This is a bit complicate because we want to make sure strings stays in the same place whatever it happens...
	*
	* So we have fixed size clusters that we try to fill as max as we can with all the strings
	*
	* A cluster can get full by two reasons: it ran out of indices or room for string storage
	*/
	struct Cluster
	{
		std::array<char, DATA_BUFFER_SIZE>			m_arNames;
		uint32_t									m_uPosition;

		std::array<NameData, MAX_NAMES>				m_arInfo;
		uint32_t									m_uInfoIndex;
	};	

	//Make sure if fits on 16 bits
	static_assert(DATA_BUFFER_SIZE <= std::numeric_limits<uint16_t>::max());

	class RNameState
	{
		public:
			RNameState();

			NameIndex RegisterName(std::string_view name);

			inline const std::string_view GetName(detail::NameIndex index) const
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
			//Dynamically allocate each cluster so when a new cluster is created, the old ones does not change ther memory location
			//This way string views are always valids
			std::vector<std::unique_ptr<Cluster>>					m_vecClusters;

			std::map<uint64_t, std::array<NameIndex, CHAIN_SIZE>>	m_mapIndex;

			size_t													m_uFirstNonFullCluster = 0;

			std::mutex												m_clLock;
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

		//Is the hash already there?
		if (it != m_mapIndex.end())
		{
			//ok, make sure we have a valid string...
			for (auto &index : it->second)
			{
				if (!index)
					break;

				auto registeredName = this->GetName(index);
				if (registeredName.compare(name) == 0)
#ifdef DCCLITE_DEBUG
					return RName{ index,  registeredName };
#else
					return RName{ index };
#endif
			}
		}

		return RName{};
	}

	NameIndex RNameState::RegisterName(std::string_view name)
	{		
		auto hash = CityHash64(name.data(), name.size());

		std::unique_lock lock{ m_clLock };

		auto it = m_mapIndex.find(hash);
		size_t hashChainIndex = 0;		

		if (it != m_mapIndex.end())
		{
			for (; hashChainIndex < it->second.size(); ++hashChainIndex)
			{
				auto index = it->second[hashChainIndex];
				if (index)
				{
					auto registeredName = this->GetName(index);
					if (registeredName.compare(name) == 0)
						return index;
				}
			
				//I do not expect a collision to happens... but just in case, check it, do not want to 
				//spend awful hours chasing a strange bug if this happens....

				//
				//Anyway, as we check it, create room for a first collision and handle it... but never tested...
				//
				//If someones besides me reads this and knows two diferent strings that generate the same hash for CityHash, please let me know....

				//OMG, we go a collision!!!
				//lets try to see this...
				for (int i = 0; i < 30; ++i)
					dcclite::Log::Warn("[RNameState::RegisterName] Hash collision {} with {}", name, this->GetName(index));

				break;
			}	

			if (hashChainIndex == it->second.size())
			{
				//Filled all the chain... sorry...
				throw std::runtime_error(fmt::format("[RNameState::RegisterName] Too many hash collisions: {} with {}", name, this->GetName(it->second[0])));
			}
		}

		Cluster *cluster = nullptr;
		size_t clusterIndex = m_uFirstNonFullCluster;

		//only update m_uFirstNonFullCluster on first interaction, to avoid not checking again clusters that have name room, but could not fit large strings
		bool first = true;

		for (;;)
		{			
			for (auto len = m_vecClusters.size(); clusterIndex < len; ++clusterIndex)
			{
				auto item = m_vecClusters[clusterIndex].get();
				if (item->m_uInfoIndex == item->m_arInfo.size())
				{
					m_uFirstNonFullCluster = first ? clusterIndex + 1 : m_uFirstNonFullCluster;
					continue;
				}

				//found a cluster with room for at least one extra index...
				cluster = item;
				break;
			}			

			//all clusters full?
			if (!cluster)
			{
				//create a new cluster...
				m_vecClusters.emplace_back(new Cluster());
				cluster = m_vecClusters.back().get();

				//update index so late we know where the cluster is
				clusterIndex = m_vecClusters.size() - 1;

				m_uFirstNonFullCluster = first ? clusterIndex : m_uFirstNonFullCluster;
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
				//cluster is full and we have more clusters? - UnitTest LimitTest should test this condition
				if (clusterIndex < m_vecClusters.size() - 1)
				{
					//do not update "m_uFirstNonFullCluster" anymore if the buffer is NOT full, we may use this space later
					first = cluster->m_uPosition >= cluster->m_arNames.size();

					//skip to next cluster and start over
					//This is the case when the cluster has free name indices, but not room for the string
					++clusterIndex;
					cluster = nullptr;					

					//try next cluster
					continue;
				}

				//we checked all clusters, so we need to allocate a new one
				m_vecClusters.emplace_back(new Cluster());
				cluster = m_vecClusters.back().get();

				clusterIndex = m_vecClusters.size() - 1;
			}
			
			//
			//Create a new index on the cluster
			cluster->m_arInfo[cluster->m_uInfoIndex].m_uPosition = cluster->m_uPosition;
			cluster->m_arInfo[cluster->m_uInfoIndex].m_uSize = static_cast<uint16_t>(len);

			const auto startPosition = cluster->m_uPosition;

			//
			//copy the string
			strncpy(&cluster->m_arNames[cluster->m_uPosition], name.data(), len);
			cluster->m_uPosition += static_cast<uint32_t>(name.size());
			cluster->m_arNames[cluster->m_uPosition++] = '\0';

			NameIndex index;
			index.m_uCluster = static_cast<uint16_t>(clusterIndex);
			index.m_uIndex = cluster->m_uInfoIndex++;

			//hash collision? 
			if (hashChainIndex > 0)
			{
				//Add to the chain...
				assert(hashChainIndex < CHAIN_SIZE);
				assert(!it->second[hashChainIndex]);

				it->second[hashChainIndex] = index;
			}
			else
			{
				assert(m_mapIndex.find(hash) == m_mapIndex.end());

				//new entry...
				m_mapIndex[hash][0] = index;
			}			

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
#ifdef DCCLITE_DEBUG
		m_svName = detail::GetState().GetName(m_stIndex);
#endif
	}

	const std::string_view RName::GetData() const
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

