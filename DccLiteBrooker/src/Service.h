#pragma once

#include <string>

class Service
{
	private:
		std::string m_strName;

	protected:
		Service(const std::string &name):
			m_strName(name)
		{
			//empty
		}

	public:
		virtual ~Service() {}

		const std::string &GetName() { return m_strName; }

};
