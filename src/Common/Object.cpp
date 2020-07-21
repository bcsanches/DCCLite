// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Object.h"
#include <sstream>

#include <fmt/format.h>

#include "Util.h"

namespace dcclite
{
	ObjectPath::ObjectPath(std::string_view str) :
		m_strPath(dcclite::StrTrim(str))
	{
		//empty
	}

	void ObjectPath::append(std::string_view other)
	{		
		other = dcclite::StrTrim(other);
		if (m_strPath.empty())
		{
			m_strPath = other;
		}
		else
		{					
			if (m_strPath.back() != '/')
			{
				m_strPath += '/';
			}

			m_strPath += other;
		}
	}

	bool ObjectPath::is_absolute() const
	{
		return m_strPath.empty() ? false : m_strPath[0] == '/';
	}

	Path_t IObject::GetPath() const
	{
		Path_t path;

		GetPath_r(path);

		return path;
	}

	IObject &IObject::GetRoot()
	{
		IObject *result = this;

		while (result->m_pParent)
			result = result->m_pParent;

		return *result;
	}

	void IObject::GetPath_r(Path_t &path) const
	{
		if (m_pParent)
		{
			m_pParent->GetPath_r(path);

			path.append(this->GetName());
		}				
		else
		{
			path = Path_t("/");
		}
	}

	void IObject::SerializeIdentification(JsonOutputStream_t &stream) const
	{
		stream.AddStringValue("name", this->GetName());
		stream.AddIntValue("internalId", reinterpret_cast<intptr_t>(this));
		stream.AddStringValue("path", this->GetPath().string());
	}

	void IObject::Serialize(JsonOutputStream_t &stream) const
	{
		this->SerializeIdentification(stream);

		stream.AddStringValue("className", this->GetTypeName());		
		stream.AddBool("isShortcut", this->IsShortcut());
		stream.AddBool("isFolder", this->IsFolder());				

		if (m_pParent)
		{
			stream.AddStringValue("parentName", m_pParent->GetName());
			stream.AddIntValue("parentId", reinterpret_cast<intptr_t>(m_pParent));
		}			
	}

	Object::Object(std::string name) :
		IObject(std::move(name))		
	{
		//empty
	}

	void Shortcut::Serialize(JsonOutputStream_t &stream) const
	{
		IObject::Serialize(stream);

		stream.AddStringValue("target", m_rTarget.GetPath().string());
	}

	FolderObject::FolderObject(std::string name):
		IObject(std::move(name))
	{
		//empty
	}

	IObject *FolderObject::AddChild(std::unique_ptr<IObject> obj)
	{
		if (obj->m_pParent)
		{			
			throw std::runtime_error(fmt::format("error: Object::AddChild(std::unique_ptr<Object> obj) object {} is a child of {}", obj->GetName(), obj->m_pParent->GetName()));
		}

		auto it = m_mapObjects.lower_bound(obj->GetName());

		if ((it != m_mapObjects.end()) && (!m_mapObjects.key_comp()(obj->GetName(), it->first)))
		{
			throw std::runtime_error(fmt::format("error: Object::AddChild(std::unique_ptr<Object> obj) object {} already exists in {}", obj->GetName(), this->GetName()));			
		}

		auto ptr = obj.get();

		m_mapObjects.insert(it, std::pair<std::string_view, std::unique_ptr<IObject>>(obj->GetName(), std::move(obj)));
		ptr->m_pParent = this;

		return ptr;
	}

	IObject *FolderObject::TryGetChild(std::string_view name)
	{
		auto it = m_mapObjects.find(name);		

		return it == m_mapObjects.end() ? nullptr : it->second.get();
	}

	IObject *FolderObject::TryResolveChild(std::string_view name)
	{
		if (auto *obj = this->TryGetChild(name))
			return (obj->IsShortcut()) ? static_cast<Shortcut*>(obj)->TryResolve() : obj;
		else
			return nullptr;		
	}

	IObject *FolderObject::TryNavigate(const Path_t &path)
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
				auto *folder = static_cast<FolderObject *>(currentNode);				

				currentNode = folder->TryResolveChild(path);

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

	std::unique_ptr<IObject> FolderObject::RemoveChild(std::string_view name) 
	{
		auto it = m_mapObjects.find(name);
		if (it == m_mapObjects.end())
			return std::unique_ptr<IObject>{};

		auto ptr = std::move(it->second);

		m_mapObjects.erase(it);

		return ptr;
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

	IObject *FolderObject::FolderEnumerator::TryGetCurrent()
	{
		assert(m_itCurrent != m_itEnd);

		auto obj = m_itCurrent->second.get();

		return obj;

		//return (obj->IsShortcut()) ? static_cast<Shortcut *>(obj)->TryResolve() : obj;		
	}	
}
