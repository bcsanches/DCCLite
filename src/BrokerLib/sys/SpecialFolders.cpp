#include "SpecialFolders.h"

namespace SpecialFolders
{
	dcclite::RName GetName(Folders id)
	{
		static dcclite::RName services{ "services" };
		static dcclite::RName cmds{ "cmds" };

		switch (id)
		{
			case Folders::ServicesId:
				return services;

			case Folders::CmdHostId:
				return cmds;

			default:
				return dcclite::RName{};
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