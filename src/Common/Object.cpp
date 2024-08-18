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

#include "Util.h"

#include "IFolderObject.h"


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

		for (;;)
		{
			auto parent = result->GetParent();
			if (!parent)
				return *result;

			result = parent;
		}
	}

	void IObject::GetPath_r(Path_t &path) const
	{		
		if (auto parent = this->GetParent())
		{
			parent->GetPath_r(path);

			path.append(this->GetNameData());
		}				
		else
		{
			path = Path_t("/");
		}
	}

	void IObject::SerializeIdentification(JsonOutputStream_t &stream) const
	{
		stream.AddStringValue("name", this->GetNameData());
		stream.AddPointerValue("internalId", this);
		stream.AddStringValue("path", this->GetPath().string());
	}

	void IObject::Serialize(JsonOutputStream_t &stream) const
	{
		this->SerializeIdentification(stream);

		stream.AddStringValue("className", this->GetTypeName());		
		stream.AddBool("isShortcut", this->IsShortcut());
		stream.AddBool("isFolder", this->IsFolder());				

		if(auto parent = this->GetParent())		
		{
			stream.AddStringValue("parentName", parent->GetNameData());
			stream.AddPointerValue("parentInternalId", parent);
		}
		else
		{
			stream.AddPointerValue("parentInternalId", nullptr);
		}
	}

	Object::Object(RName name) :
		IObject(name)		
	{
		//empty
	}

	void Shortcut::Serialize(JsonOutputStream_t &stream) const
	{
		IObject::Serialize(stream);

		stream.AddStringValue("target", m_rTarget.GetPath().string());
		stream.AddStringValue("targetClassName", m_rTarget.GetTypeName());
	}	
}
