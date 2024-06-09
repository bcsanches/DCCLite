// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#pragma once

#include "Object.h"

#include <functional>

namespace dcclite
{	
	class IFolderObject : public Object
	{
		public:
			typedef std::function<bool(IObject &child)> Visitor_t;

		public:
			explicit IFolderObject(RName name) :
				Object{ name }
			{
				//empty
			}

			virtual IObject *TryGetChild(RName name) = 0;

			IObject *TryResolveChild(RName name);

			IObject *TryNavigate(const Path_t &path);

			bool IsFolder() const noexcept override { return true; }

			virtual void VisitChildren(Visitor_t visitor) = 0;
	};	
}
