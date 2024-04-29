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

#include <vector>

#include "LitePanelLib/render/TileMapRenderer.h"

namespace dcclite::PanelEditor
{
	class Document;
	class PanelEditorApp;
	class ToolButton;

	class DocumentView
	{
		public:
			void SetDocument(Document *doc);			

			void Display(PanelEditorApp &app, const bool debugTileClipping);

		private:
			Document				*m_pclDocument = nullptr;
			const ToolButton		*m_pclCurrentTool = nullptr;
			LitePanel::MapObject	*m_pclCursorObject = nullptr;

			std::vector<LitePanel::Render::TileMapRenderer> m_vecRenderers;			

			int						m_iVisiblePanel = -1;

			bool m_fMouseHovering = false;
	};

	class DocumentWidget
	{
		public:
			virtual void Display(PanelEditorApp &app);
			virtual void Update() {};

			void SetDocument(Document *doc) 
			{
				m_clView.SetDocument(doc);
			}

			virtual ~DocumentWidget() = default;		

			[[nodiscard]] bool IsTileClipppingDebugEnabled() const noexcept
			{
				return m_fTileClippingDebug;
			}

			void EnableTileClippingDebug(bool state) noexcept
			{
				m_fTileClippingDebug = state;
			}

		private:			
			DocumentView m_clView;

			bool m_fTileClippingDebug = false;			
	};
}
