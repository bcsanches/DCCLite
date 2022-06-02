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

#include <string.h>

#define PROGMEM

class __FlashStringHelper;

inline size_t strlen_P(const __FlashStringHelper *fstr)
{
	return strlen((const char *)fstr);
}

inline int strncmp_P(const char *str1, const __FlashStringHelper *fstr2, size_t maxCount)
{
	return strncmp(str1, (const char *)fstr2, maxCount);
}

inline char *strncpy_P(char *dest, const __FlashStringHelper *fsrc, size_t count)
{
	return strncpy(dest, (const char *)fsrc, count);
}

#define F(x) (reinterpret_cast<const __FlashStringHelper *>(x))