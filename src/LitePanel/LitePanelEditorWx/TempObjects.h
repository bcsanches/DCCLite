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

#include <LitePanelLib/MapObject.h>
#include <LitePanelLib/Render/Color.h>


namespace LitePanel::Render
{
	struct ViewInfo;
}

namespace LitePanel
{
	namespace Gui
	{
		struct ViewInfo;
	}	

	class TempObject: public MapObject
	{
		public:
			TempObject(const TileCoord_t& position):
				MapObject(position)
			{
				//empty
			}			
		
			bool IsPersistent() const noexcept override
			{
				return false;
			}
	};	

	class QuadObject : public TempObject
	{
		public:
			QuadObject(const TileCoord_t& position, LitePanel::Render::Color_t color):
				TempObject{ position },
				m_tColor{color}
			{
				//empty
			}

			void Draw(Render::IRenderer &renderer, const Render::ViewInfo &viewInfo, const FloatPoint_t &tileOrigin) const override;			

		private:	
			LitePanel::Render::Color_t m_tColor;

	};
}


