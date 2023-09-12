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

#include <array>
#include <map>
#include <mutex>

#include <fmt/format.h>

#define MAX_NAMES		1024
#define MAX_NAME_LEN	1024

namespace dcclite::detail
{

	struct NameData
	{
		uint16_t m_uPosition;
		uint16_t m_uSize;
	};

	class RNameState
	{
		public:
			RNameState();

			uint32_t RegisterName(std::string_view name);

			inline std::string_view GetName(uint32_t index) const
			{
				auto &info = m_arInfo[index];

				return std::string_view{ &m_arNames[info.m_uPosition], info.m_uSize };
			}

			inline std::optional<dcclite::RName> TryGetName(std::string_view name);

		private:
			std::array<char, MAX_NAMES *MAX_NAME_LEN>	m_arNames;
			uint32_t									m_uPosition;

			std::array<NameData, MAX_NAMES>				m_arInfo;
			uint32_t									m_uInfoIndex;

			std::map<std::string_view, uint32_t>		m_mapIndex;

			std::mutex									m_clLock;
	};

	RNameState::RNameState() :
		m_uPosition{ 0 },
		m_uInfoIndex{ 0 }
	{
		//consume position 0 with null....
		this->RegisterName("null_name");
	}

	inline std::optional<dcclite::RName> RNameState::TryGetName(std::string_view name)
	{
		std::unique_lock lock{ m_clLock };

		auto it = m_mapIndex.find(name);
		if (it != m_mapIndex.end())
		{
			//name already registered?
			return RName{ it->second };
		}

		return {};
	}

	uint32_t RNameState::RegisterName(std::string_view name)
	{
		std::unique_lock lock{ m_clLock };

		auto it = m_mapIndex.find(name);
		if (it != m_mapIndex.end())
		{
			//name already registered?
			return it->second;
		}

		if (m_uInfoIndex == m_arInfo.size())
		{
			throw std::runtime_error(fmt::format("[RNameState::TryRegisterName] No room for {} on info array", name));
		}

		//plus \0
		const auto len = name.length();

		//should not be necessary, as it will not fit on array... but just in case we change this array size later and forget this...
		if (len >= std::numeric_limits<uint16_t>().max())
		{
			throw std::runtime_error(fmt::format("[RNameState::TryRegisterName] Name {} is too big", name));
		}

		if ((len + 1) + m_uPosition >= m_arNames.size())
		{
			throw std::runtime_error(fmt::format("[RNameState::TryRegisterName] No room for {} on data array", name));
		}

		m_arInfo[m_uInfoIndex].m_uPosition = m_uPosition;
		m_arInfo[m_uInfoIndex].m_uSize = static_cast<uint16_t>(len);		

		const auto startPosition = m_uPosition;

		strncpy(&m_arNames[m_uPosition], name.data(), len);
		m_uPosition += static_cast<uint32_t>(name.size());
		m_arNames[m_uPosition++] = '\0';

		m_mapIndex[std::string_view(&m_arNames[startPosition], len)] = m_uInfoIndex++;

		return m_uInfoIndex - 1;
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
		m_uNameIndex{ detail::GetState().RegisterName(name) }
	{
		//empty
	}

	std::string_view RName::GetData() const
	{
		return detail::GetState().GetName(m_uNameIndex);
	}

	std::optional<RName> RName::TryGetName(std::string_view name)
	{
		return detail::GetState().TryGetName(name);		
	}

	RName RName::GetName(std::string_view name)
	{
		auto opt = RName::TryGetName(name);

		if (!opt)
			throw std::invalid_argument(fmt::format("[RName::GetName] Name \"{}\" is not registered", name));

		return opt.value();
	}
}
