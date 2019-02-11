#include "PathUtils.h"

#include <filesystem>

static std::string g_strAppName;

void dcclite::PathUtils::SetAppName(std::string_view name)
{
	g_strAppName = name;
}

#ifndef WIN32
#error "implement me"
#endif

#include "Shlobj.h"

std::filesystem::path dcclite::PathUtils::GetAppFolder()
{
	PWSTR pFolderPath = nullptr;

	HRESULT hr = SHGetKnownFolderPath(FOLDERID_LocalAppData, 0, NULL, &pFolderPath);
	if (hr != S_OK)
	{
		throw std::runtime_error("error: dcclite::GetAppFolder cannot get CSIDL_LOCAL_APPDATA");
	}

	std::filesystem::path result(pFolderPath);
	result.append("dcclite");
	result.append(g_strAppName);

	CoTaskMemFree(pFolderPath);

	return result;
}
