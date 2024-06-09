// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "IFolderObject.h"

namespace dcclite
{
	//
	//
	// IFolderObject
	//
	//

	IObject *IFolderObject::TryResolveChild(RName name)
	{
		if (auto *obj = this->TryGetChild(name))
			return (obj->IsShortcut()) ? static_cast<Shortcut *>(obj)->TryResolve() : obj;
		else
			return nullptr;
	}

	IObject *IFolderObject::TryNavigate(const Path_t &path)
	{
		IObject *currentNode = this;

		for (auto it = path.begin(); (it != path.end()) && (currentNode); ++it)
		{
			auto path = it.ToString();

			if (path.compare("/") == 0)
			{
				currentNode = &this->GetRoot();
				continue;
			}
			else if (path.compare(".") == 0)
				continue;
			else if (path.compare("..") == 0)
			{
				currentNode = this->GetParent();

				continue;
			}
			else if (currentNode->IsFolder())
			{
				auto *folder = static_cast<IFolderObject *>(currentNode);

				currentNode = folder->TryResolveChild(RName::Create(path));

				if (!currentNode)
					return nullptr;
			}
			else
			{
				return nullptr;
			}
		}

		return currentNode;
	}	
}
