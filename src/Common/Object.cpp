#include "Object.h"

#include <cassert>
#include <sstream>

#include <fmt/format.h>

namespace dcclite
{
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
			m_pParent->GetPath_r(path);

		path.append(this->GetName());
	}

	Object::Object(std::string name) :
		IObject(std::move(name))		
	{
		//empty
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
		auto *obj = this->TryGetChild(name);
		
		return (obj && obj->IsShortcut()) ? static_cast<Shortcut*>(obj)->TryResolve() : nullptr;
	}

	IObject *FolderObject::TryNavigate(const Path_t &path)
	{
		IObject *currentNode = nullptr;

		auto it = path.begin();

		if (path.is_absolute())
		{
			currentNode = &this->GetRoot();
			++it;
		}
		else
		{
			currentNode = this;
		}

		for (; it != path.end(); ++it)
		{
			if (currentNode->IsFolder())
			{
				auto *folder = static_cast<FolderObject *>(currentNode);

				currentNode = folder->TryResolveChild(it->string());

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

		return (obj->IsShortcut()) ? static_cast<Shortcut *>(obj)->TryResolve() : obj;		
	}	
}