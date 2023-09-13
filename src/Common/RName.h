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

namespace dcclite
{
	namespace detail
	{
		class RNameState;
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

			inline RName():
				m_uNameIndex{ 0 }
			{
				//empty	
			}			

			RName(const RName &) = default;
			RName(RName &&) = default;

			RName &operator=(const RName &rhs) = default;
			RName &operator=(RName &&rhs) = default;

			inline bool operator==(const RName &rhs) const
			{
				return m_uNameIndex == rhs.m_uNameIndex;
			}

			inline bool operator!=(const RName &rhs) const
			{
				return m_uNameIndex != rhs.m_uNameIndex;
			}

			//This is not lexicographic, just for comparing indices....
			inline bool operator<(const RName &rhs) const
			{
				return m_uNameIndex < rhs.m_uNameIndex;
			}

			explicit operator bool() const
			{
				return m_uNameIndex;
			}

			std::string_view GetData() const;

		private:			
			RName(uint32_t index) :
				m_uNameIndex{ index }
			{
				//empty
			}

			friend class detail::RNameState;

		private:
			uint32_t	m_uNameIndex;
	};
}
