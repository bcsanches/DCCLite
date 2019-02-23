#pragma once

#include <filesystem>
#include <map>
#include <string>
#include <memory>

namespace dcclite
{
	class FolderObject;

	class ObjectPathConstIterator
	{
		private:
			friend class ObjectPath;

			ObjectPathConstIterator(std::string::const_iterator begin, std::string::const_iterator end):
				m_itBlockBegin(begin),
				m_itPathEnd(end)
			{
				if (begin == end)
					m_itBlockEnd = end;
				else if (*begin == '/')
				{
					m_itBlockEnd = begin + 1;
				}
			}

		public:
			typedef std::forward_iterator_tag iterator_category;
			
			ObjectPathConstIterator(const ObjectPathConstIterator &rhs) = default;

			ObjectPathConstIterator& operator=(const ObjectPathConstIterator&rhs) = default;

			bool operator==(const ObjectPathConstIterator&rhs) const
			{
				return (m_itBlockBegin == rhs.m_itBlockBegin) && (m_itBlockEnd == rhs.m_itBlockEnd);
			}

			bool operator!=(const ObjectPathConstIterator &rhs) const
			{
				return (m_itBlockBegin != rhs.m_itBlockBegin) || (m_itBlockEnd != rhs.m_itBlockEnd);
			}

			ObjectPathConstIterator& operator++()
			{

			}

			ObjectPathConstIterator operator++(int) //optional
			{
				auto temp = *this;

				++*this;

				return temp;
			}

		private:
			std::string::const_iterator m_itBlockBegin;
			std::string::const_iterator m_itBlockEnd;
			std::string::const_iterator m_itPathEnd;
	};

	class ObjectPath
	{
		public:
			

		public:
			ObjectPath() noexcept;
			ObjectPath(const ObjectPath &rhs) :
				m_strPath(rhs.m_strPath)
			{
				//empty
			}

			ObjectPath(ObjectPath &&rhs) :
				m_strPath(std::move(rhs.m_strPath))
			{
				//empty
			}

			void append(std::string_view other);

			bool is_absolute() const;

		private:
			std::string m_strPath;
	};

	typedef std::filesystem::path Path_t;

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
			virtual bool IsFolder() const { return false; }

			Path_t GetPath() const;
			IObject &GetRoot();

		private:
			const std::string m_strName;

			void GetPath_r(Path_t &path) const;

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

			IObject *TryNavigate(const Path_t &path);

			virtual bool IsFolder() const { return true; }

			inline FolderEnumerator GetEnumerator() 
			{
				return FolderEnumerator(m_mapObjects.begin(), m_mapObjects.end());
			}

		private:
			Container_t m_mapObjects;
	};	
}
