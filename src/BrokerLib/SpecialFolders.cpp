#include "SpecialFolders.h"

namespace SpecialFolders
{
	const char *GetName(Folders id)
	{
		switch (id)
		{
			case Folders::ServicesId:
				return "services";

			case Folders::CmdHostId:
				return "cmds";

			default:
				return nullptr;
		}
	}

	const char *GetPath(Folders id)
	{
		switch (id)
		{
		case Folders::ServicesId:
			return "/services";

		case Folders::CmdHostId:
			return "/cmds";

		default:
			return nullptr;
		}
	}
}