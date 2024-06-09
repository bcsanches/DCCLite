// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "FolderObject.h"

#include "FmtUtils.h"

namespace dcclite
{	
	
	FolderObject::FolderObject(RName name):
		IFolderObject(name)
	{
		//empty
	}

	IObject *FolderObject::AddChild(std::unique_ptr<Object> obj)
	{
		if (obj->GetParent())
		{			
			throw std::runtime_error(fmt::format("error: Object::AddChild(std::unique_ptr<Object> obj) object {} is a child of {}", obj->GetName(), obj->m_pParent->GetName()));
		}

		auto it = m_mapObjects.lower_bound(obj->GetName());

		if ((it != m_mapObjects.end()) && (!m_mapObjects.key_comp()(obj->GetName(), it->first)))
		{
			throw std::runtime_error(fmt::format("error: Object::AddChild(std::unique_ptr<Object> obj) object {} already exists in {}", obj->GetName(), this->GetName()));			
		}

		auto ptr = obj.get();

		m_mapObjects.insert(it, std::pair<RName, std::unique_ptr<Object>>(obj->GetName(), std::move(obj)));
		ptr->m_pParent = this;

		return ptr;
	}

	IObject *FolderObject::TryGetChild(RName name)
	{
		auto it = m_mapObjects.find(name);		

		return it == m_mapObjects.end() ? nullptr : it->second.get();
	}	

	void FolderObject::RemoveAllChildren()
	{
		m_mapObjects.clear();
	}

	std::unique_ptr<IObject> FolderObject::RemoveChild(RName name) 
	{
		auto it = m_mapObjects.find(name);
		if (it == m_mapObjects.end())
			return std::unique_ptr<IObject>{};

		auto ptr = std::move(it->second);

		m_mapObjects.erase(it);

		return ptr;
	}

	void FolderObject::VisitChildren(Visitor_t visitor)
	{
		auto enumerator{ this->GetEnumerator() };

		while(enumerator.MoveNext())		
		{
			visitor(*enumerator.GetCurrent());
		}
	}

	FolderObject::FolderEnumerator::FolderEnumerator(FolderObject::Iterator_t begin, FolderObject::Iterator_t end):
		m_itBegin(begin),
		m_itEnd(end),
		m_itCurrent(begin),
		m_fFirst(true)
	{
		//empty
	}

	bool FolderObject::FolderEnumerator::MoveNext()
	{
		if (m_fFirst)
		{
			m_fFirst = false;			
		}
		else if(m_itCurrent != m_itEnd)
		{		
			++m_itCurrent;
		}

		return m_itCurrent != m_itEnd;
	}

	IObject *FolderObject::FolderEnumerator::GetCurrent()
	{
		assert(m_itCurrent != m_itEnd);

		auto obj = m_itCurrent->second.get();

		return obj;

		//return (obj->IsShortcut()) ? static_cast<Shortcut *>(obj)->TryResolve() : obj;		
	}
}
