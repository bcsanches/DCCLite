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

#include "IFolderObject.h"

#include <map>

namespace dcclite
{	
	class FolderObject : public IFolderObject
	{
		public:
			typedef std::map<RName, std::unique_ptr<Object>> Container_t;
			
		public:
			explicit FolderObject(RName name);

			virtual IObject *AddChild(std::unique_ptr<Object> obj);
			
			std::unique_ptr<IObject> RemoveChild(RName name);
			void RemoveAllChildren();

			IObject *TryGetChild(RName name) override;		

			void ConstVisitChildren(ConstVisitor_t visitor) const override;
			void VisitChildren(Visitor_t visitor) override;

			void KillerVisitChildren(Visitor_t visitor);
			
			const char *GetTypeName() const noexcept override
			{
				return "dcclite::FolderObject";
			}

			void Serialize(JsonOutputStream_t &stream) const override
			{
				IObject::Serialize(stream);

				//nothing
			}

		private:
			Container_t m_mapObjects;
	};	
}
