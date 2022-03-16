// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Misc.h"

#include "Parser.h"

#include <stdexcept>
#include <sstream>

namespace dcclite
{
	int ParseNumber(const char *str)
	{
		dcclite::Parser parser{ str };

		int adr;
		if (parser.GetNumber(adr) != dcclite::Tokens::NUMBER)
		{
			std::stringstream stream;

			stream << "[dcclite::ParseNumber] String " << str << " does not contains a valid number";

			throw std::runtime_error(stream.str());
		}

		return adr;
	}
}

