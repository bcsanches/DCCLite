#pragma once

namespace dcclite
{
	class Guid
	{		
		public:
			//it is public, but access it at your own risk.
			union
			{
				uint8_t m_bId[8];
				uint64_t m_bBigId[2];
			};

			inline Guid()
			{
				m_bBigId[0] = m_bBigId[1] = 0;
			}

			inline bool IsNull() const noexcept
			{
				return (m_bBigId[0] == 0) && (m_bBigId[1] == 0);
			}

			inline bool operator==(const Guid &g) const noexcept
			{
				return ((m_bBigId[0] == g.m_bBigId[0]) && (m_bBigId[1] == g.m_bBigId[1]));
			}

			inline bool operator!=(const Guid &g) const noexcept
			{
				return ((m_bBigId[0] != g.m_bBigId[0]) || (m_bBigId[1] != g.m_bBigId[1]));
			}
	};

}

