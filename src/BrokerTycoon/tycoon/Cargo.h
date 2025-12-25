// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include <dcclite/Object.h>

namespace dcclite::broker::tycoon
{
	class Cargo : public INamedItem
	{
		public:
			Cargo(RName name) :
				INamedItem{ name }
			{
				//empty
			}

			Cargo(Cargo &&rhs) noexcept = default;
			Cargo &operator=(Cargo &&rhs) noexcept = default;

		private:
	};
}
