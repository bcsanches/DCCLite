// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Sha1.h"

#include "Util.h"

///////////////////////////////////////////////////////////////////////////////
//
// SHA1.cpp : Console App to hash files using SHA-1.
//
//            Version 1.0 -- 2011, October 29th
//
// Copyright 2011, by Giovanni Dicanio <giovanni.dicanio@gmail.com>
//
///////////////////////////////////////////////////////////////////////////////
std::string dcclite::Sha1::ToString() const
{
	//
	// Get the hash digest string from hash value buffer.
	//

	// Each byte --> 2 hex chars
	std::string hashDigest;
	hashDigest.resize(sizeof(mData) * 2);

	// Upper-case hex digits
	static const char hexDigits[] = "0123456789ABCDEF";

	// Index to current character in destination string
	size_t currChar = 0;

	// For each byte in the hash value buffer
	for (size_t i = 0; i < sizeof(mData); ++i)
	{
		// high nibble
		hashDigest[currChar] = hexDigits[(mData[i] & 0xF0) >> 4];
		++currChar;

		// low nibble
		hashDigest[currChar] = hexDigits[(mData[i] & 0x0F)];
		++currChar;
	}

	return hashDigest;
}

bool dcclite::Sha1::TryLoadFromString(std::string_view str)
{
	return TryHexStrToBinary(mData, sizeof(mData), str);
}
