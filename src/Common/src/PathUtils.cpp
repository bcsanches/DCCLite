#include "PathUtils.h"

#include <filesystem>

static std::string g_strAppName;

void dcclite::PathUtils::SetAppName(std::string_view name)
{
	g_strAppName = name;
}

