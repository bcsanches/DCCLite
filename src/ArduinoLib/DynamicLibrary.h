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

#include <memory>
#include <string>
#include <string_view>


class DynamicLibrary
{
	public:
		DynamicLibrary();
		~DynamicLibrary();

		void Load(const std::string_view name);
		void Unload();

		void *TryGetSymbol(const char *name);
		void *GetSymbol(const char *name);

	private:
		void RaiseException(const char *module, const char *dll);

	private:
		std::string	m_strName;

		typedef std::unique_ptr<void, void(*)(void *)> ModuleHandle_t;
		ModuleHandle_t m_upHandle;
};
