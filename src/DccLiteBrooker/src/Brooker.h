#pragma once

#include <map>
#include <memory>
#include <string>

#include "Object.h"
#include "Service.h"

class Brooker
{
	private:	
		dcclite::Object m_clRoot;

	public:
		Brooker();

		void LoadConfig(const char *fileName);

		void Update();
};
