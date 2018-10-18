#include "Object.h"

#include <sstream>

Object::Object(std::string name) :
	m_strName(name)
{
	//empty
}

void Object::AddChild(std::unique_ptr<Object> obj)
{
	auto it = m_mapObjects.lower_bound(obj->m_strName);

	if ((it != m_mapObjects.end()) && (!m_mapObjects.key_comp()(obj->m_strName, it->first)))
	{
		std::stringstream stream;

		stream << "error: Object::AddChild(std::unique_ptr<Object> obj) object " << obj->m_strName << " already exists in " << this->m_strName;

		throw std::runtime_error(stream.str());
	}

	m_mapObjects.insert(it, std::make_pair(obj->m_strName, std::move(obj)));
}
