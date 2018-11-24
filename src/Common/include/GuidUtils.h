#pragma once

#include <string>

namespace dcclite
{
	class Guid;

	extern Guid GuidCreate();

	extern std::string GuidToString(const dcclite::Guid &g);
}
