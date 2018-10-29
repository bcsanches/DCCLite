#pragma once

#include <memory>
#include <string_view>


class DynamicLibrary
{
	public:
		DynamicLibrary();
		~DynamicLibrary();

		void Load(const std::string_view name);

		void *TryGetSymbol(const char *name);
		void *GetSymbol(const char *name);

	private:
		void RaiseException(const char *module, const char *dll);

	private:
		std::string	m_strName;

		typedef std::unique_ptr<void, void(*)(void *)> ModuleHandle_t;
		ModuleHandle_t m_upHandle;
};
