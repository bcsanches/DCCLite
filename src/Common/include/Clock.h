#pragma once

#include <chrono>

namespace dcclite
{
	class Clock
	{
		public:
			Clock();

			bool Tick(std::chrono::milliseconds min = std::chrono::milliseconds{ 1 });

			std::chrono::milliseconds Delta();

			std::chrono::milliseconds Total();

		private:
			typedef std::chrono::high_resolution_clock DefaultClock_t;

			std::chrono::time_point<DefaultClock_t> m_StartTime;
			std::chrono::time_point<DefaultClock_t> m_CurrentTime;
			std::chrono::time_point<DefaultClock_t> m_PreviousTime;
	};
}
