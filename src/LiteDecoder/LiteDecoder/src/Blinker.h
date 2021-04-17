// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#ifndef BLINKER_H
#define BLINKER_H

namespace Blinker
{
	enum class Animations
	{
		OK,
		ERROR
	};

	extern void Init();

	extern void Update();

	extern void Play(Animations animation);
}

#endif