#pragma once

#include <map>
#include <memory>
#include <string>

#include "Object.h"
#include "Service.h"

namespace dcclite
{
	class Clock;
}

class Brooker
{
	private:	
		dcclite::FolderObject m_clRoot;

	public:
		Brooker();

		void LoadConfig(const char *fileName);

		void Update(const dcclite::Clock &clock);
};
