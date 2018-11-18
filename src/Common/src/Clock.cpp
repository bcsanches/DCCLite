#include "Clock.h"

namespace dcclite
{	
	Clock::Clock():
		m_StartTime(DefaultClock_t::now())		
	{
		m_CurrentTime = m_StartTime;
		m_PreviousTime = m_StartTime;
	}

	bool Clock::Tick(std::chrono::milliseconds min)
	{
		auto t = DefaultClock_t::now();

		if (t - m_CurrentTime < min)
			return false;

		m_PreviousTime = m_CurrentTime;
		m_CurrentTime = t;		

		return true;
	}

	std::chrono::milliseconds Clock::Delta()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(m_CurrentTime - m_PreviousTime);
	}

	std::chrono::milliseconds Clock::Total()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(m_CurrentTime - m_StartTime);
	}
}
