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
			typedef std::map<RName, std::unique_ptr<IObject>> Container_t;
			typedef Container_t::iterator Iterator_t;

			class FolderEnumerator
			{
				public:
					FolderEnumerator(Iterator_t begin, Iterator_t end);

					bool MoveNext();

					IObject *GetCurrent();

					template<typename T>
					inline T *GetCurrent()
					{
						return static_cast<T*>(GetCurrent());
					}

				private:
					FolderObject::Iterator_t m_itBegin, m_itEnd, m_itCurrent;
					bool m_fFirst;
			};

		public:
			explicit FolderObject(RName name);

			virtual IObject *AddChild(std::unique_ptr<IObject> obj);
			
			std::unique_ptr<IObject> RemoveChild(RName name);
			void RemoveAllChildren();

			IObject *TryGetChild(RName name) override;			

			inline FolderEnumerator GetEnumerator() 
			{
				return FolderEnumerator(m_mapObjects.begin(), m_mapObjects.end());
			}

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
