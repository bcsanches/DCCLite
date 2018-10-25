#include "Object.h"

#include <cassert>
#include <sstream>

namespace dcclite
{
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
			std::stringstream stream;

			stream << "error: Object::AddChild(std::unique_ptr<Object> obj) object " << obj->GetName() << " is a child of " << obj->m_pParent->GetName();

			throw std::runtime_error(stream.str());
		}

		auto it = m_mapObjects.lower_bound(obj->GetName());

		if ((it != m_mapObjects.end()) && (!m_mapObjects.key_comp()(obj->GetName(), it->first)))
		{
			std::stringstream stream;

			stream << "error: Object::AddChild(std::unique_ptr<Object> obj) object " << obj->GetName() << " already exists in " << this->GetName();

			throw std::runtime_error(stream.str());
		}

		m_mapObjects.insert(it, std::make_pair(obj->GetName(), std::move(obj)));
		obj->m_pParent = this;

		return obj.get();
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
