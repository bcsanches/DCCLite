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

#include "FileSystem.h"

#include "LitePanelLib/Panel.h"
#include "Util.h"

namespace dcclite::panel_editor
{	
	class Document
	{
		public:
			Document();

			DCCLITE_DISABLE_CLASS_COPY_AND_MOVE(Document);

			void New();

			void Save();
			void SaveAs(dcclite::fs::path &path);

			inline void MarkDirty() noexcept
			{
				m_fDirty = true;
			}

			[[nodiscard]] bool IsDirty() const noexcept
			{
				return m_fDirty;
			}

			[[nodiscard]] bool IsExistingDoc() const noexcept
			{
				return m_fExistingDoc;
			}

			const LitePanel::Panel *GetPanels() const noexcept
			{
				return &m_vecPanels[0];
			}

			size_t GetNumPanels() const noexcept
			{
				return m_vecPanels.size();
			}			

		private:			
			dcclite::fs::path	m_pthFileName;

			std::vector<LitePanel::Panel> m_vecPanels;

			bool m_fDirty = false;
			bool m_fExistingDoc = false;
	};
}
