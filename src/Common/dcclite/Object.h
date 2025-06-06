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

#include <algorithm>
#include <stdexcept>

#include <string>

#include <JsonCreator/Object.h>
#include <JsonCreator/StringWriter.h>

#include "RName.h"

namespace dcclite
{
	class IFolderObject;

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
				else
				{
					m_itBlockEnd = std::find(begin, end, '/');
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
				if (m_itBlockBegin == m_itPathEnd)
					return *this;

				m_itBlockBegin = m_itBlockEnd;
				if (m_itBlockBegin == m_itPathEnd)
					return *this;

				//for root paths, m_itBlockBegin will not point to /, so we must check
				if (*m_itBlockBegin == '/')
				{
					//skip current /
					++m_itBlockBegin;
				}				

				m_itBlockEnd = std::find(m_itBlockBegin, m_itPathEnd, '/');

				return *this;
			}

			ObjectPathConstIterator operator++(int) //optional
			{
				auto temp = *this;

				++*this;

				return temp;
			}

			inline std::string ToString() const
			{
				return std::string(m_itBlockBegin, m_itBlockEnd);
			}

		private:
			std::string::const_iterator m_itBlockBegin;
			std::string::const_iterator m_itBlockEnd;
			std::string::const_iterator m_itPathEnd;
	};

	class ObjectPath
	{		
		public:
			ObjectPath() = default;

			ObjectPath(const ObjectPath &rhs) = default;				
			ObjectPath(ObjectPath &&rhs) = default;

			explicit ObjectPath(std::string_view str);

			ObjectPath &operator=(const ObjectPath &rhs) = default;
			ObjectPath &operator=(ObjectPath &&rhs) = default;

			void append(std::string_view other);

			bool is_absolute() const;
			bool is_relative() const
			{
				return !is_absolute();
			}

			ObjectPathConstIterator begin() const
			{
				return ObjectPathConstIterator(m_strPath.begin(), m_strPath.end());
			}

			ObjectPathConstIterator end() const
			{
				return ObjectPathConstIterator(m_strPath.end(), m_strPath.end());
			}

			inline const std::string &string() const
			{
				return m_strPath;
			}

		private:
			std::string m_strPath;
	};

	typedef ObjectPath Path_t;
	typedef JsonCreator::Object<JsonCreator::StringWriter> JsonOutputStream_t;

	class IItem
	{
		public:
			virtual ~IItem()
			{
				//empty
			}

			virtual void Serialize(JsonOutputStream_t &stream) const = 0;
	};

	class IObject: public IItem
	{		
		public:
			explicit IObject(RName name) :
				m_rnName{ name }				
			{
				if (!name)
				{
					throw std::invalid_argument("RName cannot be null");
				}
			}

			virtual ~IObject() = default;

			IObject(const IObject &rhs) = default;
			IObject(IObject &&rhs) = default;

			inline IObject &operator=(IObject &&) = default;			
			IObject &operator=(const IObject &) = default;

			inline RName GetName() const noexcept { return m_rnName; }
			inline std::string_view GetNameData() const noexcept { return m_rnName.GetData(); }

			virtual bool IsShortcut() const noexcept { return false; }
			virtual bool IsFolder() const noexcept { return false; }

			Path_t GetPath() const;
			IObject &GetRoot();

			virtual const char *GetTypeName() const noexcept = 0;

			void SerializeIdentification(JsonOutputStream_t &stream) const;

			void Serialize(JsonOutputStream_t &stream) const override;

			virtual IFolderObject *GetParent() const noexcept = 0;

		private:
			RName m_rnName;

			void GetPath_r(Path_t &path) const;		
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
		friend class FolderObject;

		public:
			explicit Object(RName name);	

			Object(const Object &) = delete;

			IFolderObject *GetParent() const noexcept override
			{
				return m_pParent;
			}

			const char *GetTypeName() const noexcept override
			{
				return "Object";
			}

		private:
			IFolderObject *m_pParent = nullptr;
	};	

	class Shortcut : public Object
	{
		public:
			Shortcut(RName name, IObject &target) :
				Object(name),
				m_rTarget(target)
			{
				//empty
			}

			IObject *TryResolve()
			{
				return &m_rTarget;
			}

			bool IsShortcut() const noexcept override { return true; }

			const char *GetTypeName() const noexcept override
			{
				return "dcclite::Shortcut";
			}			

			void Serialize(JsonOutputStream_t &stream) const override;

		private:
			IObject &m_rTarget;			
	};	
}
