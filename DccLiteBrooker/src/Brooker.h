#pragma once

#include <map>
#include <memory>
#include <string>

#include "Service.h"

class Brooker
{
	private:
		std::map<std::string, std::unique_ptr<Service>> m_mapServices;	

	public:
		Brooker();

		void LoadConfig(const char *fileName);
};
