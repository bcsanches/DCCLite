#pragma once

#include <assert.h>
#include <memory>

namespace dcclite
{
	template <typename T, typename ...ARGS>
	class ClassInfo
	{
		public:
			typedef std::unique_ptr<T> (*CreateObjectProc_t)(const ClassInfo<T, ARGS...> &objClass, ARGS...);

		private:
			const char *m_pszName;
			CreateObjectProc_t m_pfnProc;

			class ClassInfo<T, ARGS...> *m_pNext;
			static inline class ClassInfo<T, ARGS...> *g_pHead = nullptr;

		public:
			ClassInfo(const char *name, CreateObjectProc_t proc) :
				m_pszName(name),
				m_pfnProc(proc)
			{
				assert((name != nullptr) && strlen(name));
				assert(m_pfnProc);

				//register this type on linked list
				m_pNext = g_pHead;
				g_pHead = this;
			}

			std::unique_ptr<T> CreateUnique(ARGS... args)
			{
				assert(m_pfnProc);

				return m_pfnProc(*this, args...);
			}

			static std::unique_ptr<T> TryProduce(const char *className, ARGS... args)
			{
				for (auto ptr = g_pHead; ptr; ptr = ptr->m_pNext)
				{
					if (strcmp(ptr->m_pszName, className) == 0)
					{
						return ptr->CreateUnique(args...);
					}
				}

				return nullptr;
			}
	};
}