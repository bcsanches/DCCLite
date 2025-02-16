// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "DynamicLibrary.h"

#include <iostream>

#include <fmt/format.h>

#include <dcclite/FileSystem.h>
#include <dcclite/Util.h>

//based on https://github.com/bcsanches/phobos3d/blob/master/src/Base/DynamicLibrary.cpp

#ifndef WIN32
#include <dlfcn.h>

inline void *OpenLib(const char *name)
{
	return dlopen(name, RTLD_LAZY);
}

inline void CloseLib(void* handle)
{
	dlclose(handle);
}

#define GetLibSymbol dlsym

inline std::string GetOpenLibLastErrorMessage()
{
	auto errorMsg = dlerror();

	return std::string(errorMsg);
}

#elif defined WIN32
#include <windows.h>

inline void CloseLib(void *handle)
{
	FreeLibrary(static_cast<HMODULE>(handle));
}

#define OpenLib LoadLibraryW
inline void *GetLibSymbol(void *handle, const char *name)
{
	return GetProcAddress(static_cast<HMODULE>(handle), name);
}

//win32 has some consistency and always use GetLastError, so call the GetSystemLastErrorMessage
inline std::string GetOpenLibLastErrorMessage()
{
	return dcclite::GetSystemLastErrorMessage();
}

#endif

DynamicLibrary::DynamicLibrary() :
	m_upHandle(nullptr, &CloseLib)
{
	//empty
}

DynamicLibrary::~DynamicLibrary()
{
	//empty
}

void DynamicLibrary::Load(const std::string_view name)
{
	m_strName = name;

	dcclite::fs::path path(name);	

#ifdef PH_LINUX
	Path path(tmp);
	path.SetExtension(".so");
	tmp = path.GetStr();
#endif

	m_upHandle.reset(OpenLib(path.native().c_str()));

	if (m_upHandle == NULL)
	{
		this->RaiseException("DynamicLibrary::Load", m_strName.c_str());
	}
}

void *DynamicLibrary::TryGetSymbol(const char *name)
{
	return GetLibSymbol(m_upHandle.get(), name);
}

void *DynamicLibrary::GetSymbol(const char *name)
{
	void *ptr = this->TryGetSymbol(name);
	if (ptr == NULL)
	{
		this->RaiseException("DynamicLibrary::GetSymbol", name);
	}

	return ptr;
}

void DynamicLibrary::RaiseException(const char *module, const char *dll)
{
	auto errMsg = GetOpenLibLastErrorMessage();

	throw std::logic_error(fmt::format("DynamicLibrary[{}] {} not found item {}", module, errMsg, dll));
}
