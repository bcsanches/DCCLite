#pragma once

#include <map>
#include <memory>
#include <string>

#include "Object.h"
#include "Project.h"
#include "Service.h"

namespace dcclite
{
	class Clock;
}

class Brooker
{
	private:	
		dcclite::FolderObject m_clRoot;

		Project m_clProject;

	private:

		void LoadConfig();

	public:
		Brooker(std::filesystem::path projectPath);

		void Update(const dcclite::Clock &clock);
};