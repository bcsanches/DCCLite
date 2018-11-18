#include "GuidUtils.h"

#include <Windows.h>

#include "fmt/format.h"

#include "Guid.h"


dcclite::Guid dcclite::GuidCreate()
{
	GUID g;

	auto h = CoCreateGuid(&g);
	if (h != S_OK)
	{
		throw std::runtime_error(fmt::format("[GUID::Create] CoCreateGuid failed: {}", h));
	}

	dcclite::Guid guid;

	memcpy(guid.m_bId, &g.Data1, sizeof(g.Data1));
	memcpy(guid.m_bId + 4, &g.Data2, sizeof(g.Data2));
	memcpy(guid.m_bId + 8, &g.Data3, sizeof(g.Data3));
	memcpy(guid.m_bId + 12, &g.Data4, sizeof(g.Data4));

	return guid;
}
