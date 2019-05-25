#include "SpecialFolders.h"

namespace SpecialFolders
{
	const char *GetName(Folders id)
	{
		switch (id)
		{
			case ServicesFolderId:
				return "services";

			case CmdHostFolderId:
				return "cmds";

			default:
				return nullptr;
		}
	}

	const char *GetPath(Folders id)
	{
		switch (id)
		{
		case ServicesFolderId:
			return "/services";

		case CmdHostFolderId:
			return "/cmds";

		default:
			return nullptr;
		}
	}
}