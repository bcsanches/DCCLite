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
#include <filesystem>

#include <fmt/format.h>

//based on https://github.com/bcsanches/phobos3d/blob/master/src/Base/DynamicLibrary.cpp

#ifdef PH_LINUX
#include <dlfcn.h>
#define CloseLib dlclose

inline void *OpenLib(const char *name)
{
	return dlopen(name, RTLD_LAZY);
}

#define GetLibSymbol dlsym

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

	std::filesystem::path path(name);	

#ifdef PH_LINUX
	Path path(tmp);
	path.SetExtension(".so");
	tmp = path.GetStr();
#endif

	m_upHandle.reset(OpenLib(path.native().c_str()));

	if (m_upHandle == NULL)
	{
		this->RaiseException("DynamicLibrary_c::Load", m_strName.c_str());
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
		this->RaiseException("DynamicLibrary_c::GetSymbol", name);
	}

	return ptr;
}

void DynamicLibrary::RaiseException(const char *module, const char *dll)
{
#if defined PH_LINUX
	String_t ret = dlerror();

	PH_RAISE(NATIVE_API_FAILED_EXCEPTION, module, ret);
#elif defined WIN32
	LPVOID lpMsgBuf;
	FormatMessage(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		GetLastError(),
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0,
		NULL
	);

	std::string ret = fmt::format("DynamicLibrary[{}] {} not found item {}", module, lpMsgBuf, dll);

	// Free the buffer.
	LocalFree(lpMsgBuf);
		
	throw std::logic_error(ret);
#endif
}
