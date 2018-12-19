#pragma once

#include <map>
#include <string>
#include <memory>

namespace dcclite
{
	class FolderObject;

	class IObject
	{
		friend class FolderObject;

		public:
			IObject(std::string name) :
				m_strName(std::move(name)),
				m_pParent(nullptr)
			{
				//empty
			}

			virtual ~IObject()
			{
				//empty
			}

			inline std::string_view GetName() const { return m_strName; }

			virtual bool IsShortcut() const { return false; }

		private:
			const std::string m_strName;

		protected:			
			FolderObject *m_pParent;
	};

#if 0
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
			GenericObject<T> *m_pParent;

			Container_t m_mapObjects;
	};
#endif

	class Object: public IObject
	{
		public:
			Object(std::string name);			
	};	

	class Shortcut : public IObject
	{
		public:
			Shortcut(std::string name, IObject &target) :
				IObject(std::move(name)),
				m_rTarget(target)
			{
				//empty
			}

			IObject *TryResolve()
			{
				return &m_rTarget;
			}

			virtual bool IsShortcut() const { return true; }

		private:
			IObject &m_rTarget;			
	};

	class FolderObject : public IObject
	{
		public:
			typedef std::map<std::string_view, std::unique_ptr<IObject>> Container_t;
			typename typedef Container_t::iterator Iterator_t;

			class FolderEnumerator
			{
				public:
					FolderEnumerator(Iterator_t begin, Iterator_t end);

					bool MoveNext();

					IObject *TryGetCurrent();

					template<typename T>
					inline T *TryGetCurrent()
					{
						return static_cast<T*>(TryGetCurrent());
					}

				private:
					FolderObject::Iterator_t m_itBegin, m_itEnd, m_itCurrent;
					bool m_fFirst;
			};

		public:
			FolderObject(std::string name);

			IObject *AddChild(std::unique_ptr<IObject> obj);
			
			std::unique_ptr<IObject> RemoveChild(std::string_view name);

			IObject *TryGetChild(std::string_view name);

			IObject *TryResolveChild(std::string_view name);

			inline FolderEnumerator GetEnumerator() 
			{
				return FolderEnumerator(m_mapObjects.begin(), m_mapObjects.end());
			}

		private:
			Container_t m_mapObjects;
	};	
}
