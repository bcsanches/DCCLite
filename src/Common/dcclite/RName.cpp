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

#define CHAIN_SIZE		2

constexpr auto DATA_BUFFER_SIZE = 1024;

namespace dcclite::detail
{	
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
				assert(index.m_uIndex < m_vecNames.size());

				return m_vecNames[index.m_uIndex];				
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
				info.m_uRoomLeft = static_cast<uint32_t>(cluster->m_arNames.size()) - cluster->m_uPosition;				

				return info;
			}

			uint32_t FindNameCluster(NameIndexType_t index);
			std::pair<uint32_t, uint32_t> FindClusterInfo(NameIndexType_t index);

			std::vector<RName> GetAll();

			void ForceNewCluster();
			

		private:
			//Dynamically allocate each cluster so when a new cluster is created, the old ones does not change ther memory location
			//This way string views are always valids
			std::vector<std::unique_ptr<Cluster>>					m_vecClusters;
			std::vector<std::string_view>							m_vecNames;

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
				{
#ifdef DCCLITE_DEBUG
					return RName{ index,  registeredName };
#else
					return RName{ index };
#endif
				}
				else
				{
					//OMG, we got a collision!!!
					//lets try to see this on the log...
					for (int i = 0; i < 30; ++i)
						dcclite::Log::Warn("[RNameState::RegisterName] Hash collision {} with {}", name, this->GetName(index));
				}
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

				//OMG, we got a collision!!!
				//lets try to see this on the log...
				for (int i = 0; i < 30; ++i)
					dcclite::Log::Warn("[RNameState::RegisterName] Hash collision {} with {}", name, this->GetName(index));
								
				break;
			}	

			if (hashChainIndex == it->second.size())
			{
				//Filled all the chain... sorry... should I use those chars ASCII as lottery numbers? Or perhaps the hash?
				throw std::runtime_error(fmt::format("[RNameState::RegisterName] Too many hash collisions: {} with {}", name, this->GetName(it->second[0])));
			}
		}
		
		//
		//need to register a new name
		//
				
		//plus \0
		const auto nameLength = name.length();

		if (nameLength + 1 >= DATA_BUFFER_SIZE)
		{
			throw std::runtime_error(fmt::format("[RNameState::TryRegisterName] Name {} is too big", name));
		}
		
		//should never happens... but...
		if (m_vecNames.size() >= std::numeric_limits<uint32_t>::max())
		{
			throw std::runtime_error(fmt::format("[RNameState::TryRegisterName] Too many names! Cannot register {}", name));
		}

		//only update m_uFirstNonFullCluster on first interaction, to avoid not checking again clusters that are filled up
		bool first = true;
		Cluster *cluster = nullptr;

		for (;;)
		{			
			for (size_t clusterIndex = m_uFirstNonFullCluster, clusterVecLen = m_vecClusters.size(); clusterIndex < clusterVecLen; ++clusterIndex)
			{
				auto item = m_vecClusters[clusterIndex].get();

				//does the data fits on the buffer?
				if ((nameLength + 1) + item->m_uPosition >= item->m_arNames.size())
				{
					//should have at least 2 bytes
					if ((item->m_uPosition >= item->m_arNames.size() - 2) && (first))
					{
						m_uFirstNonFullCluster = clusterIndex + 1;
					}

					first = false;
					continue;
				}

				//found a cluster with room 
				cluster = item;
				break;
			}

			//all clusters full?
			if (!cluster)
			{
				//create a new cluster...
				m_vecClusters.emplace_back(new Cluster());
				cluster = m_vecClusters.back().get();							
			}			
					
			const auto startPosition = cluster->m_uPosition;

			//
			//copy the string
			strncpy(&cluster->m_arNames[cluster->m_uPosition], name.data(), nameLength);
			cluster->m_uPosition += static_cast<uint32_t>(name.size());
			cluster->m_arNames[cluster->m_uPosition++] = '\0';

			m_vecNames.emplace_back(&cluster->m_arNames[startPosition], nameLength);

			NameIndex index;
			index.m_uIndex = static_cast<uint16_t>(m_vecNames.size() - 1);

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

	void RNameState::ForceNewCluster()
	{
		m_uFirstNonFullCluster = m_vecClusters.size();
	}

	std::vector<RName> RNameState::GetAll()
	{
		std::vector<RName> result;

		result.reserve(m_mapIndex.size());

		for (auto it : m_mapIndex)
		{
			auto index = it.second;

#ifdef DCCLITE_DEBUG
			result.push_back(RName{ index[0], this->GetName(index[0])});
#else
			result.push_back(RName{ index[0]});
#endif			

			if (index[1])
			{
#ifdef DCCLITE_DEBUG
				result.push_back(RName{ index[1], this->GetName(index[1]) });
#else
				result.push_back(RName{ index[1] });
#endif			
			}
		}

		return result;
	}

	uint32_t RNameState::FindNameCluster(NameIndexType_t index)
	{
		auto name = m_vecNames[index];

		for (size_t i = 0, len = m_vecClusters.size(); i < len; ++i)
		{
			if (name.data() < &m_vecClusters[i]->m_arNames[0])
				continue;

			if (name.data() >= &m_vecClusters[i]->m_arNames[0] + m_vecClusters[i]->m_arNames.size())
				continue;

			//if(((name.data() + len) < (&m_vecClusters[i]->m_arNames[0] + m_vecClusters[i]->m_arNames.size())))
				return static_cast<uint32_t>(i);
		}

		throw std::runtime_error(fmt::format("[RNameState::FindNameCluster] cluster for {} not found, where is it?", name));
	}

	std::pair<uint32_t, uint32_t> RNameState::FindClusterInfo(NameIndexType_t index)
	{
		auto name = m_vecNames[index];

		auto cluster = this->FindNameCluster(index);

		auto position = static_cast<uint32_t>(name.data() - &m_vecClusters[cluster]->m_arNames[0]);

		return std::make_pair(cluster, position);
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

	uint32_t RName::FindCluster() const
	{
		return detail::GetState().FindNameCluster(this->m_stIndex.m_uIndex);
	}

	std::pair<uint32_t, uint32_t> RName::FindClusterInfo() const
	{
		return detail::GetState().FindClusterInfo(this->m_stIndex.m_uIndex);
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

	std::vector<RName> RName_GetAll()
	{
		auto &state = GetState();

		return state.GetAll();
	}

	void RName_ForceNewCluster()
	{
		auto &state = GetState();

		state.ForceNewCluster();
	}
}

