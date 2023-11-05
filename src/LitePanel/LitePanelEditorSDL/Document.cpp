// Copyright (C) 2019 - Bruno Sanches. See the COPYRIGHT
// file at the top-level directory of this distribution.
// 
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.
// 
// This Source Code Form is "Incompatible With Secondary Licenses", as
// defined by the Mozilla Public License, v. 2.0.

#include "Document.h"

#include <stdexcept>

#include <fmt/format.h>

namespace dcclite::panel_editor
{
	Document::Document()
	{
		//empty
	}

	void Document::Save()
	{
		if (!m_fExistingDoc)
		{
			throw std::logic_error(fmt::format("[Document::Save] Cannot save a document without a name, did you call SetFileName?"));
		}
		
		m_fDirty = false;
	}

	void Document::SaveAs(dcclite::fs::path &path)
	{
		m_pthFileName = path;
		m_fExistingDoc = true;

		this->Save();
	}
}
