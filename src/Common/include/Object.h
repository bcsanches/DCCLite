#pragma once

#include <map>
#include <string>
#include <memory>

namespace dcclite
{
	class IObject
	{
		public:
			IObject(std::string name) :
				m_strName(std::move(name))
			{
				//empty
			}

			inline const std::string &GetName() const { return m_strName; }

		private:
			const std::string m_strName;
	};

	template <typename T>
	class GenericObject: public IObject
	{
		public:
			typedef std::map<std::string_view, std::unique_ptr<T>> Container_t;
			typename typedef Container_t::iterator Iterator_t;

		public:
			inline GenericObject(std::string name) :
				IObject(std::move(name)),
				m_pParent(nullptr)
			{
				//empty
			}			
		
			void AddChild(std::unique_ptr<T> obj)
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
			}

			inline Iterator_t begin()
			{
				return m_mapObjects.begin();
			}

			inline Iterator_t end()
			{
				return m_mapObjects.end();
			}

		private:			
			IObject *m_pParent;

			Container_t m_mapObjects;
	};

	class Object: public GenericObject<Object>
	{
		public:
			Object(std::string name);			
	};
}
