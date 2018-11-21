#pragma once

#include <chrono>

namespace dcclite
{
	class Clock
	{
		public:
			typedef std::chrono::high_resolution_clock DefaultClock_t;
			typedef std::chrono::time_point<DefaultClock_t> TimePoint_t;

			Clock();

			bool Tick(std::chrono::milliseconds min = std::chrono::milliseconds{ 1 });

			inline std::chrono::milliseconds Delta() const
			{
				return std::chrono::duration_cast<std::chrono::milliseconds>(m_CurrentTime - m_PreviousTime);
			}

			inline std::chrono::milliseconds Total() const
			{
				return std::chrono::duration_cast<std::chrono::milliseconds>(m_CurrentTime - m_StartTime);
			}

			inline TimePoint_t Now() const noexcept
			{
				return m_CurrentTime;
			}

		private:			
			TimePoint_t m_StartTime;
			TimePoint_t m_CurrentTime;
			TimePoint_t m_PreviousTime;
	};
}
